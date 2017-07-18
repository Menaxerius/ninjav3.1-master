#include "Block.hpp"
#include "Reward.hpp"

#include "BaseHandler.hpp"
#include "BlockCache.hpp"
#include "config.hpp"
#include "mining_info.hpp"

#include "DORM/Transaction.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"
#include "push_notification.hpp"

#include "pthread_np_shim.hpp"


const time_t PUSH_COOLOFF_PERIOD = 10 * 60;		// how long between push notifications, in seconds
time_t reward_push_cooloff_timeout = 0;

const time_t PAY_FAILURE_COOLOFF_PERIOD = 10 * 60; 	// how long after a pay failure before trying again, in seconds
time_t reward_pay_cooloff_timeout = 0;


class trivial_exception: public std::runtime_error {
	public:
		trivial_exception(): std::runtime_error("nothing to see here") {};
};



void reward_payer() {
    pthread_set_name_np(pthread_self(), "reward_payer");

    std::cout << ftime() << "Miner reward payer started" << std::endl;

	BurstCoin burst(BURST_SERVERS);

	while( !BaseHandler::time_to_die ) {
		sleep(1);

		uint64_t latest_blockID = BlockCache::latest_blockID;

		// we need DB connection from here on
		try {
			DORM::DB::check_connection();
		} catch (const DORM::DB::connection_issue &e) {
			// DB being hammered by miners - try again in a moment
			continue;
		}

		// after a suitable delay, generate rewards for blocks we've won
		Block blocks;
		blocks.before_blockID( latest_blockID - MIN_PAYOUT_BLOCK_DELAY + 1);
		blocks.has_been_shared( false );
		blocks.is_our_block( true );
		blocks.search();

		while( auto block = blocks.result() ) {
			std::cout << ftime() << "Generating rewards for block " << block->blockID() << std::endl;
			block->reward_miners();
		}

		bool no_unconfirmed_rewards = true;		// this is for later where we might slurp pool fees

		// we need to wait at least one block before transactions show up
		Reward unconfirmed_rewards;
		unconfirmed_rewards.is_paid(true);
		unconfirmed_rewards.is_confirmed(false);
		unconfirmed_rewards.paid_before_block_id( latest_blockID - 1 );
		unconfirmed_rewards.search();

		// multiple rewards can share a single BURST payout transaction ID
		// so store results so we can traverse them multiple times without hammering database
		std::vector< std::unique_ptr<Reward> > cached_unconfirmed_rewards;

		while( auto reward = unconfirmed_rewards.result() )
			cached_unconfirmed_rewards.push_back( std::move(reward) );

		for (int i=0; i<cached_unconfirmed_rewards.size(); ++i) {
			auto &reward = cached_unconfirmed_rewards[i];

			// already dealt with?
			if ( !reward->is_paid() || reward->is_confirmed() )
				continue;

			// make a note of original tx_id
			const uint64_t orig_tx_id = reward->tx_id();

			CryptoCoinTx tx;
			tx.tx_id = std::to_string( orig_tx_id );

			int confirmations;

			try {
				// we might as well fetch the whole transaction info as some later code needs it
				tx = burst.get_transaction( tx );

				confirmations = tx.confirmations;
			} catch (const CryptoCoins::unknown_transaction &e) {
				// -1 means unknown transaction
				confirmations = -1;
			} catch (const CryptoCoins::server_issue &e) {
				// something went wrong - cool-off for a while
				continue;
			}

			if (confirmations == -1) {
				std::cout << ftime() << "Blockchain seems to have lost tx id " << reward->tx_id() <<
						" to account " << BurstCoin::accountID_to_RS_string( reward->accountID() ) << ", block " << reward->paid_at_block_id() << std::endl;

				// find transactions sharing same tx id and reset those
				for (int j=i; j<cached_unconfirmed_rewards.size(); ++j) {
					auto &other_reward = cached_unconfirmed_rewards[j];

					if (other_reward->tx_id() != orig_tx_id)
						continue;

					other_reward->is_paid(false);
					other_reward->undef_tx_id();
					other_reward->undef_paid_at_block_id();
					other_reward->save();
				}
			} else if ( confirmations >= 10 ) {
				// fully confirmed

				// find transactions sharing same tx id and update those
				for (int j=i; j<cached_unconfirmed_rewards.size(); ++j) {
					auto &other_reward = cached_unconfirmed_rewards[j];

					if (other_reward->tx_id() != orig_tx_id)
						continue;

					other_reward->is_confirmed(true);
					other_reward->save();
				}
			} else {
				// not sure yet
				no_unconfirmed_rewards = false;
			}
		}

		// did we have a payment failure recently?
		if ( time(nullptr) < reward_pay_cooloff_timeout )
			continue;


		// now check for unpaid rewards that can be sent out

		// we want to collate rewards per account so only do one transaction per account
		// we only sum rewards that are at least MIN_PAYOUT_BLOCK_DELAY old
		// we don't payout if the sum is below MINIMUM_PAYOUT
		//		...unless the oldest reward is over MAX_PAYOUT_BLOCK_DELAY

		// first make a list of accounts with unpaid rewards
		Reward unpaid_rewards;
		unpaid_rewards.is_paid(false);
		unpaid_rewards.is_confirmed(false);
		unpaid_rewards.before_blockID( latest_blockID - MIN_PAYOUT_BLOCK_DELAY + 1 );
		unpaid_rewards.search();

		std::map< uint64_t, std::vector<std::unique_ptr<Reward>> > rewards_by_accountID;
		while( auto reward = unpaid_rewards.result() )
			rewards_by_accountID[ reward->accountID() ].push_back( std::move(reward) );

		uint64_t deferred_amount = 0;

		// now cycle through each account
		for( const auto &map_it : rewards_by_accountID ) {
			if (BaseHandler::time_to_die)
				return;	// quicker process exit

			const uint64_t accountID = map_it.first;
			const auto &rewards = map_it.second;
			bool ok_to_pay = false;
			uint64_t sum_amount = 0;

			DORM::Transaction db_tx_guard;

			for( auto &reward : rewards ) {
				// check whether miner has been waiting for ages
                // Don't sum if not reah max delay
				if ( reward->defined_blockID() && reward->blockID() < latest_blockID - MAX_PAYOUT_BLOCK_DELAY ){
                    ok_to_pay = true;
                    sum_amount += reward->amount();
                }
			}

			if ( sum_amount >= MINIMUM_PAYOUT ){
                ok_to_pay = true;
            }

			if ( !ok_to_pay ) {
				deferred_amount += sum_amount + PAYMENT_SEND_FEE;
				continue;
			}

			// actually send amount
			CryptoCoinTx tx;
			tx.sender = OUR_ACCOUNT_RS;
			tx.encoded_passphrase = OUR_ACCOUNT_PASSPHRASE;
			tx.fee_inclusive = false;
			tx.fee = PAYMENT_SEND_FEE;

			std::string recipientRS = BurstCoin::accountID_to_RS_string( accountID );
			tx.recipient_amounts.push_back( { recipientRS, sum_amount } );

			try {
				if ( !burst.send_transaction(tx) ) {
					std::cout << ftime() << "Couldn't send reward of " << burst.pretty_amount(sum_amount) << " to " << recipientRS << std::endl;

					// this is somewhat unexpected - notify someone
					const time_t now = time(nullptr);

					if (now > reward_push_cooloff_timeout) {
						reward_push_cooloff_timeout = now + PUSH_COOLOFF_PERIOD;
						push_notification(PUSH_URL, PUSH_KEY, PUSH_RECIPIENT, "Can't send pool miner's reward");
					}

					reward_pay_cooloff_timeout = now + PAY_FAILURE_COOLOFF_PERIOD;
					break;
				}
			} catch (const CryptoCoins::server_issue &e) {
				// something went wrong - cool-off for a while
				reward_pay_cooloff_timeout = time(nullptr) + PAY_FAILURE_COOLOFF_PERIOD;
				break;
			}

			// sent OK- update all the rewards for this account
			for( const auto &reward : rewards ) {
				reward->is_paid(true);
				reward->paid_at_block_id(latest_blockID);
				reward->tx_id( std::stoull(tx.tx_id) );
				reward->save();
			}

			db_tx_guard.commit();

			std::cout << ftime() << "Sent reward of " << burst.pretty_amount(sum_amount) << " to account " << recipientRS << ", tx " << tx.tx_id << std::endl;

			sleep(1);
		}


		// did we have a payment failure recently?
		if ( time(nullptr) < reward_pay_cooloff_timeout )
			continue;


		// if there are no blocks for which we haven't received the reward
		// then we can probably send excess balance to pool's fee account
		Block more_blocks;
		more_blocks.is_our_block(true);
		more_blocks.after_blockID( latest_blockID - MIN_PAYOUT_BLOCK_DELAY);

		if ( no_unconfirmed_rewards && !more_blocks.present() ) {
			bool need_cooloff = false;

			try {
				JSON account_json( burst.get_account(OUR_ACCOUNT_RS) );

				if ( account_json.null("unconfirmedBalanceNQT") )
					throw trivial_exception();	// no balance for some reason

				uint64_t balance = account_json.get_uint64("unconfirmedBalanceNQT");

				if (balance < deferred_amount + MINIMUM_PAYOUT + PAYMENT_SEND_FEE)
					throw trivial_exception();	// don't sweep trivial amount of leftover funds to fee account - save the fees

				// slurpy slurp slurp
				CryptoCoinTx tx;
				tx.sender = OUR_ACCOUNT_RS;
				tx.encoded_passphrase = OUR_ACCOUNT_PASSPHRASE;
				tx.fee_inclusive = false;
				tx.fee = PAYMENT_SEND_FEE;

				tx.recipient_amounts.push_back( { POOL_FEE_ACCOUNT_RS , balance - deferred_amount - PAYMENT_SEND_FEE } );

				if ( !burst.send_transaction(tx) ) {
					// this is somewhat unexpected - notify someone
					const time_t now = time(nullptr);

					if (now > reward_push_cooloff_timeout) {
						reward_push_cooloff_timeout = now + PUSH_COOLOFF_PERIOD;
						push_notification(PUSH_URL, PUSH_KEY, PUSH_RECIPIENT, "Can't slurp excess pool balance into pool fee account");
					}

					reward_pay_cooloff_timeout = now + PAY_FAILURE_COOLOFF_PERIOD;
					continue;
				}

				std::cout << ftime() << "Leftover balance: " << burst.pretty_amount(balance) << ", of which ring-fenced/deferred: " << burst.pretty_amount(deferred_amount) << std::endl;
				std::cout << ftime() << "So " << burst.pretty_amount(balance - deferred_amount) << " (minus fee) swept to pool fee account " << POOL_FEE_ACCOUNT_RS << ", tx id " << tx.tx_id << std::endl;
			} catch (const CryptoCoins::server_issue &e) {
				need_cooloff = true;
			} catch (const JSON::parse_error &e) {
				need_cooloff = true;
			} catch(const trivial_exception &e) {
				need_cooloff = true;
			}

			if (need_cooloff) {
				// something went wrong - cool-off for a while
				reward_pay_cooloff_timeout = time(nullptr) + PAY_FAILURE_COOLOFF_PERIOD;
				continue;
			}
		}
	}
}

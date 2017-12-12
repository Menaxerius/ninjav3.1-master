#include "Block.hpp"
#include "Reward.hpp"
#include "Fee.hpp"

#include "WS/updates.hpp"

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

const time_t FAILURE_COOLOFF_PERIOD = 2 * 60; 	// how long after a pay failure before trying again, in seconds
time_t reward_failure_cooloff_timeout = 0;

const time_t BALANCE_CHECK_COOLOFF_PERIOD = 2 * 60;	// how long between non-critical pool balance checks, in seconds
time_t last_balance_check_timeout = 0;


static int get_transaction_confirmations(BurstCoin &burst, const uint64_t tx_id) {
	CryptoCoinTx tx;
	tx.tx_id = std::to_string( tx_id );

	try {
		// we might as well fetch the whole transaction info as some later code needs it
		tx = burst.get_transaction( tx );

		return tx.confirmations;
	} catch (const CryptoCoins::unknown_transaction &e) {
		// Unlike bitcoin, BURST tries hard not to discard unconfirmed transactions
		// so we shouldn't really ever have this situation unless the original node died badly

		// check unconfirmed transactionID list from nodes as a last resort before giving up
		JSON unconfirmed_json;

		try {
			unconfirmed_json = burst.get_unconfirmed_transactionIDs(OUR_ACCOUNT_ID);
		} catch (const CryptoCoins::server_issue &e) {
			// something went wrong - cool-off for a while
			reward_failure_cooloff_timeout = time(nullptr) + FAILURE_COOLOFF_PERIOD;

			return 0;
		}

		if ( !unconfirmed_json.exists("unconfirmedTransactionIds") ) {
			// should exist, even if empty
			reward_failure_cooloff_timeout = time(nullptr) + FAILURE_COOLOFF_PERIOD;

			return 0;
		}

		JSON_Array unconfirmed_json_array = unconfirmed_json.get_array("unconfirmedTransactionIds");
		const int n_transactions = unconfirmed_json_array.size();

		for (int j=0; j<n_transactions; ++j)
			if ( unconfirmed_json_array.get_uint64(j) == tx_id ) {
				// unconfirmed, but not lost

				return 0;
			}

		// pretty likely to be lost
		// -1 means unknown transaction

		return -1;
	} catch (const CryptoCoins::server_issue &e) {
		// something went wrong - cool-off for a while
		reward_failure_cooloff_timeout = time(nullptr) + FAILURE_COOLOFF_PERIOD;

		return 0;
	}
}


static bool have_unconfirmed_fees(BurstCoin &burst, const uint64_t latest_blockID) {
	bool result = false;

	// Check to see if there are any unconfirmed pool fee transactions
	Fee unconfirmed_fees;
	unconfirmed_fees.is_paid(true);
	unconfirmed_fees.is_confirmed(false);
	unconfirmed_fees.search();

	while( auto fee = unconfirmed_fees.result() ) {
		if (BaseHandler::time_to_die)
			return true;	// quicker process exit

		if ( latest_blockID - fee->paid_at_block_id() <= MIN_PAYOUT_BLOCK_DELAY ) {
			result = true;
			continue;
		}

		const int confirmations = get_transaction_confirmations( burst, fee->tx_id() );

		if (confirmations == -1) {
			// lost tx - mark as unpaid, some other code will resend
			std::cout << ftime() << "Blockchain seems to have lost pool fee tx " << fee->tx_id()
				<< " paid at block " << fee->paid_at_block_id() << std::endl;

			fee->undef_tx_id();
			fee->undef_paid_at_block_id();
			fee->is_paid(false);
			fee->save();
		}

		if (confirmations < 10) {
			result = true;
			continue;
		}

		// confirmed!
		fee->is_confirmed(true);
		fee->save();
	}

	return result;
}


static bool have_unconfirmed_rewards(BurstCoin &burst, const uint64_t latest_blockID) {
	bool result = false;

	Reward unconfirmed_rewards;
	unconfirmed_rewards.is_paid(true);
	unconfirmed_rewards.is_confirmed(false);
	unconfirmed_rewards.search();

	// multiple rewards can share a single BURST payout transaction ID
	// so cache results so we can traverse them multiple times without hammering database
	std::vector< std::unique_ptr<Reward> > cached_unconfirmed_rewards;

	while( auto reward = unconfirmed_rewards.result() )
		cached_unconfirmed_rewards.push_back( std::move(reward) );

	for (int i=0; i<cached_unconfirmed_rewards.size(); ++i) {
		if (BaseHandler::time_to_die)
			return true;	// quicker process exit

		auto &reward = cached_unconfirmed_rewards[i];

		// already dealt with?
		if ( !reward->is_paid() || reward->is_confirmed() )
			continue;

		// we need to wait a few blocks because tx might take a while to make into a block
		// especially if original node that handled tx hasn't broadcast it out to other nodes
		if ( latest_blockID - reward->paid_at_block_id() <= MIN_PAYOUT_BLOCK_DELAY ) {
			result = true;
			continue;
		}

		// make a note of original tx_id
		const uint64_t orig_tx_id = reward->tx_id();

		const int confirmations = get_transaction_confirmations( burst, reward->tx_id() );

		if (confirmations == -1) {
			std::cout << ftime() << "Blockchain seems to have lost reward tx " << reward->tx_id()
				<< " to account " << BurstCoin::accountID_to_RS_string( reward->accountID() )
				<< ", paid at block " << reward->paid_at_block_id() << std::endl;

			// find transactions sharing same tx id and reset those too
			for (int j=i; j<cached_unconfirmed_rewards.size(); ++j) {
				auto &other_reward = cached_unconfirmed_rewards[j];

				if (other_reward->tx_id() != orig_tx_id)
					continue;

				other_reward->is_paid(false);
				other_reward->undef_tx_id();
				other_reward->undef_paid_at_block_id();
				other_reward->save();
			}
		}

		if ( confirmations < 10 ) {
			result = true;
			continue;
		}

		// fully confirmed

		// find transactions sharing same tx id and update those too
		for (int j=i; j<cached_unconfirmed_rewards.size(); ++j) {
			auto &other_reward = cached_unconfirmed_rewards[j];

			if (other_reward->tx_id() != orig_tx_id)
				continue;

			other_reward->is_confirmed(true);
			other_reward->save();
		}
	}

	return result;
}


void send_rewards(BurstCoin &burst, const uint64_t latest_blockID, uint64_t &deferred_amount, uint64_t &queued_amount, const bool calculate_only) {
	// now check for unpaid rewards that can be sent out

	// we want to collate rewards per account so only do one transaction per account
	// we only sum rewards that are at least MIN_PAYOUT_BLOCK_DELAY old
	// we don't payout if the sum is below MINIMUM_PAYOUT
	//		...unless the oldest reward is over MAX_PAYOUT_BLOCK_DELAY

	// first make a list of accounts with unpaid rewards
	Reward unpaid_rewards;
	unpaid_rewards.is_paid(false);
	unpaid_rewards.is_confirmed(false);
	unpaid_rewards.oldest_per_account_first(true);
	unpaid_rewards.search();

	// collate rewards per account
	std::map< uint64_t, std::vector<std::unique_ptr<Reward>> > rewards_by_accountID;
	while( auto reward = unpaid_rewards.result() )
		rewards_by_accountID[ reward->accountID() ].push_back( std::move(reward) );


	// now cycle through each account
	for( const auto &map_it : rewards_by_accountID ) {
		if (BaseHandler::time_to_die)
			return;	// quicker process exit

            const uint64_t accountID = map_it.first;
            const auto &rewards = map_it.second;
            bool ok_to_pay = false;
            uint64_t sum_amount = 0;
            // Sum of delayed payouts older than MAX_PAYOUT_BLOCK_DELAY
            uint64_t late_sum_amount = 0;

		#ifdef DEBUG_REWARD_PAYER
			std::cout << "[REWARD] accountID: " << accountID << std::endl;
		#endif

		for( auto &reward : rewards ) {
			#ifdef DEBUG_REWARD_PAYER
				std::cout << "[REWARD] rewardID: " << reward->rewardID() << ", blockID: " << reward->blockID() << ", amount: " << burst.pretty_amount( reward->amount() ) << std::endl;
			#endif

			// check whether miner has been waiting for ages
			// sum delayed payouts seperately.
			if (reward->defined_blockID() && reward->blockID() < latest_blockID - MAX_PAYOUT_BLOCK_DELAY) {
				late_sum_amount = reward->amount();
			} else {
				sum_amount += reward->amount();
			}

			#ifdef DEBUG_REWARD_PAYER
				std::cout << "[REWARD] ok-to-pay: " << (ok_to_pay ? "yes" : "no") << ", sum-amount: " << burst.pretty_amount(sum_amount) << ", deferred_amount: " << burst.pretty_amount(deferred_amount) << std::endl;
			#endif
		}

		if ( sum_amount + late_sum_amount >= MINIMUM_PAYOUT ){
			ok_to_pay = true;
			sum_amount += late_sum_amount;
		} else if(late_sum_amount >= MINIMUM_DEFERRED_PAYOUT) {
			// If there is delayed payouts passing the minimum deferred pay out limit pay just those
			ok_to_pay = true;
			deferred_amount += sum_amount;
			sum_amount = late_sum_amount;
		} else {
			// Defer all
			sum_amount += late_sum_amount;
		}

		if ( !ok_to_pay ) {
			deferred_amount += sum_amount + PAYMENT_SEND_FEE;

			#ifdef DEBUG_REWARD_PAYER
				std::cout << "[REWARD] FINAL (NO PAY) sum-amount: " << burst.pretty_amount(sum_amount) << ", deferred_amount: " << burst.pretty_amount(deferred_amount) << std::endl;
			#endif

			continue;
		}

		queued_amount += sum_amount;

		#ifdef DEBUG_REWARD_PAYER
			std::cout << "[REWARD] FINAL (OK PAY) sum-amount: " << burst.pretty_amount(sum_amount) << ", deferred_amount: " << burst.pretty_amount(deferred_amount)
					<< ", queued_amount: " << burst.pretty_amount(queued_amount) << std::endl;
		#endif

		// multiple uses further on inside this for() loop
		const time_t now = time(nullptr);

		// starting up or did we have a failure recently?
		if ( calculate_only || now < reward_failure_cooloff_timeout )
			continue;	// don't actually send reward but allow fall-through to balance updates below

		#ifdef DEBUG_REWARD_PAYER
			std::cout << "[REWARD] attempting to pay " << burst.pretty_amount(sum_amount) << std::endl;
		#endif

		// actually send amount
		DORM::Transaction db_tx_guard;

		CryptoCoinTx tx;
		tx.sender = OUR_ACCOUNT_RS;
		tx.encoded_passphrase = OUR_ACCOUNT_PASSPHRASE;
		tx.fee_inclusive = true;
		tx.fee = PAYMENT_SEND_FEE;

		std::string recipientRS = BurstCoin::accountID_to_RS_string( accountID );
		tx.recipient_amounts.push_back( { recipientRS, sum_amount } );

		try {
			if ( !burst.send_transaction(tx) ) {
				std::cout << ftime() << "Couldn't send reward of " << burst.pretty_amount(sum_amount) << " to " << recipientRS << std::endl;

				// add this to deferred total as it's wasn't really paid and we don't want it swept to pool fee account
				queued_amount += sum_amount;

				// this is somewhat unexpected - notify someone
				if (now > reward_push_cooloff_timeout) {
					reward_push_cooloff_timeout = now + PUSH_COOLOFF_PERIOD;
					push_notification(PUSH_URL, PUSH_KEY, PUSH_RECIPIENT, "Can't send pool miner's reward");
				}

				reward_failure_cooloff_timeout = now + FAILURE_COOLOFF_PERIOD;
				continue;
			}
		} catch (const CryptoCoins::server_issue &e) {
			// something went wrong - cool-off for a while
			reward_failure_cooloff_timeout = now + FAILURE_COOLOFF_PERIOD;
			continue;
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

		sleep(3);
	}
}


void reward_payer() {
    pthread_set_name_np(pthread_self(), "reward_payer");

    std::cout << ftime() << "Miner reward payer started" << std::endl;

	BurstCoin burst(BURST_SERVERS);

	bool initial_scan = true;

	while( !BaseHandler::time_to_die ) {
		sleep(3);

		uint64_t latest_blockID = BlockCache::latest_blockID;
		if (latest_blockID == 0)
			continue;	// still starting up

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
			std::cout << ftime() << "Pre-generation deferred total: " << burst.pretty_amount(BlockCache::deferred_total) << ", pool balance: " << burst.pretty_amount(BlockCache::pool_balance) << std::endl;
			block->reward_miners();
		}

		const bool no_unconfirmed_fees = ! have_unconfirmed_fees(burst, latest_blockID);
		const bool no_unconfirmed_rewards = ! have_unconfirmed_rewards(burst, latest_blockID);

		uint64_t deferred_amount = 0;
		uint64_t queued_amount = 0;

		send_rewards(burst, latest_blockID, deferred_amount, queued_amount, initial_scan);	// NB: deferred_amount and queued_amount are side-effected

		const bool no_recent_failure = time(nullptr) > reward_failure_cooloff_timeout;

		// we can update deferred_amount metric now
		BlockCache::deferred_total = deferred_amount;

		// if there are no blocks for which we haven't received the reward
		// then we can probably send excess balance to pool's fee account
		Block more_blocks;
		more_blocks.is_our_block(true);
		more_blocks.after_blockID( latest_blockID - MIN_PAYOUT_BLOCK_DELAY);

		#ifdef DEBUG_REWARD_PAYER
			std::cout << "[REWARD] recent failure: " << (no_recent_failure ? "no" : "yes")
				<< ", unconfirmed fees: " << (no_unconfirmed_fees ? "no" : "yes" )
				<< ", unconfirmed rewards: " << (no_unconfirmed_rewards ? "no" : "yes" )
				<< ", pending blocks: " << (more_blocks.present() ? "yes" : "no") << std::endl;
		#endif

		const bool balance_sweep_possible = no_recent_failure && no_unconfirmed_fees && no_unconfirmed_rewards && !more_blocks.present();	// hoping short-circuit evaluation can save some DB calls here

		if ( !balance_sweep_possible && time(nullptr) < last_balance_check_timeout )
			continue;	// no need to hammer nodes for latest balance as it isn't critical and likely hasn't changed

		// grab latest pool balance
		uint64_t balance = 0;
		try {
			JSON account_json( burst.get_account(OUR_ACCOUNT_RS) );

			if ( account_json.null("unconfirmedBalanceNQT") )
				continue;	// no balance for some reason

			balance = account_json.get_uint64("unconfirmedBalanceNQT");
		} catch (const CryptoCoins::server_issue &e) {
			continue;
		} catch (const JSON::parse_error &e) {
			continue;
		}

		BlockCache::pool_balance = balance;
		last_balance_check_timeout = time(nullptr) + BALANCE_CHECK_COOLOFF_PERIOD;

		if (initial_scan) {
			std::cout << ftime() << "Initial deferred total: " << burst.pretty_amount(deferred_amount) << ", pool balance: " << burst.pretty_amount(balance) << std::endl;
			// notify websockets so they display updated values
			Handlers::WS::updates::wake_up();
			initial_scan = false;
			continue;
		}

		#ifdef DEBUG_REWARD_PAYER
			std::cout << "[REWARD] deferred total: " << burst.pretty_amount(deferred_amount) << ", queued amount: " << burst.pretty_amount(queued_amount) << ", pool balance: " << burst.pretty_amount(balance) << std::endl;
		#endif

		// NOTE: can't tidy to "balance - queued_amount - deferred_amount < MINIMUM_PAYOUT" here
		// because (balance - queued_amount - deferred_amount) could underflow to a very large +ve integer
		const uint64_t ringfenced_amount = queued_amount + deferred_amount;

		if (balance < ringfenced_amount + MINIMUM_PAYOUT)
			continue;	// don't sweep trivial amount of leftover funds to pool fee account - saves transaction fees

		if ( balance_sweep_possible ) {
			bool need_cooloff = false;

			#ifdef DEBUG_REWARD_PAYER
				std::cout << "[REWARD] Leftover balance: " << burst.pretty_amount(balance) << ", of which ring-fenced/deferred: " << burst.pretty_amount(ringfenced_amount) << std::endl;
				std::cout << "[REWARD] So " << burst.pretty_amount(balance - ringfenced_amount) << " (minus fee) would be swept to pool fee account " << POOL_FEE_ACCOUNT_RS << std::endl;
			#endif

			try {
				// sweep remaining balance to pool fee account
				DORM::Transaction db_tx_guard;

				Fee fee;
				fee.amount( balance - ringfenced_amount );
				fee.is_paid(true);
				fee.is_confirmed(false);
				fee.paid_at_block_id(latest_blockID);

				CryptoCoinTx tx;
				tx.sender = OUR_ACCOUNT_RS;
				tx.encoded_passphrase = OUR_ACCOUNT_PASSPHRASE;
				tx.fee_inclusive = true;
				tx.fee = PAYMENT_SEND_FEE;

				tx.recipient_amounts.push_back( { POOL_FEE_ACCOUNT_RS, fee.amount() } );

				if ( !burst.send_transaction(tx) ) {
					// this is somewhat unexpected - notify someone
					const time_t now = time(nullptr);

					if (now > reward_push_cooloff_timeout) {
						reward_push_cooloff_timeout = now + PUSH_COOLOFF_PERIOD;
						push_notification(PUSH_URL, PUSH_KEY, PUSH_RECIPIENT, "Can't sweep excess pool balance into pool fee account");
					}

					reward_failure_cooloff_timeout = now + FAILURE_COOLOFF_PERIOD;
					continue;
				}

				fee.tx_id( std::stoull(tx.tx_id) );
				fee.save();

				db_tx_guard.commit();

				std::cout << ftime() << "Leftover balance: " << burst.pretty_amount(balance) << ", of which ring-fenced/deferred: " << burst.pretty_amount(ringfenced_amount) << std::endl;
				std::cout << ftime() << "So " << burst.pretty_amount(balance - ringfenced_amount) << " (minus fee) swept to pool fee account " << POOL_FEE_ACCOUNT_RS << ", tx id " << tx.tx_id << std::endl;
			} catch (const CryptoCoins::server_issue &e) {
				need_cooloff = true;
			}

			if (need_cooloff) {
				// something went wrong - cool-off for a while
				reward_failure_cooloff_timeout = time(nullptr) + FAILURE_COOLOFF_PERIOD;
				continue;
			}
		}
	}
}

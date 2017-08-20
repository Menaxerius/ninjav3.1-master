#include "Share.hpp"
#include "Bonus.hpp"
#include "Reward.hpp"

#include "DORM/Transaction.hpp"

#include "BlockCache.hpp"
#include "BaseHandler.hpp"
#include "config.hpp"
#include "mining_info.hpp"

#include "BurstCoin.hpp"
#include "ftime.hpp"
#include "push_notification.hpp"

#include "pthread_np_shim.hpp"

#include <ctime>

const time_t PUSH_COOLOFF_PERIOD = 10 * 60;		// how long between push notifications, in seconds
time_t bonus_push_cooloff_timeout = 0;

const time_t PAY_FAILURE_COOLOFF_PERIOD = 10 * 60; 	// how long after a pay failure before trying again, in seconds
time_t bonus_pay_cooloff_timeout = 0;

time_t transactions_since_when = 0;


void bonus_processor() {
    pthread_set_name_np(pthread_self(), "bonus_processor");

    std::cout << ftime() << "Bonus processor started" << std::endl;

	BurstCoin burst(BURST_SERVERS);

	while(!BaseHandler::time_to_die) {
		sleep(2);

		// did we have a payment failure recently?
		if ( time(nullptr) < bonus_pay_cooloff_timeout )
			continue;

        // we need DB connection from here on
        // Check db connection
        short counter = 0;
        bool is_continue = false;
        while (true){
            try{
                DORM::DB::check_connection();
                break;
            } catch (const DORM::DB::connection_issue &e) {
                // DB being hammered by miners - try again in a moment
                std::cerr  << ftime() << "[bonus_processor::bonus_processor] Too many connections! " << e.getErrorCode() << ": " << e.what() << std::endl;
                is_continue = true;
                break;
            } catch(const sql::SQLException &e) {
                // Could not connect to db.
                std::cerr  << ftime() << "[bonus_processor::bonus_processor] " << e.what() << std::endl;
                std::cerr << ftime() << "[bonus_processor::bonus_processor] Trying to connect in a moment. Attempt: " << counter + 1 <<  std::endl;
                sleep(1);
            }
            ++counter;
            if(counter + 1 == DB_CONNECTION_ATTEMPT_COUNT){
                std::cerr << ftime() << "[bonus_processor::bonus_processor] DB connect failed..." << std::endl;
                throw;
            }
        }
        if(is_continue){
            continue;
        }

		// if we have just started up, try to carry on from last known bonus
		if (transactions_since_when == 0) {
			Bonus bonuses;
			bonuses.newest_first(true);

			auto newest_bonus = bonuses.load();

			if (newest_bonus) {
				// -1 because there may be more transactions at the same timestamp
				transactions_since_when = newest_bonus->seen_when() - 1;

				std::cout << ftime() << "Bonus processor ignoring transactions before " << asctime(gmtime( &transactions_since_when)) << std::endl;
			}
		}

		// check bonus balance
		std::vector<CryptoCoinTx> new_transactions;

		try {
			new_transactions = burst.get_recent_transactions( BONUS_ACCOUNT_RS, transactions_since_when, false );
		} catch (const CryptoCoins::server_issue &e) {
			// server issue
			continue;
		}

		if ( new_transactions.size() > 0 ) {
			const uint64_t latest_blockID = BlockCache::latest_blockID;

			for(int i=0; i<new_transactions.size(); i++) {
				CryptoCoinTx &tx = new_transactions[i];

				// update cutoff timestamp so we don't process the same ones over and over
				transactions_since_when = tx.unix_timestamp;

				// ignore transactions with no actual fund transfer
				if ( tx.recipient_amounts.size() == 0 )
					continue;

				const auto &recipient_amount = tx.recipient_amounts[0];

				// ignore zero/small amount transactions
				if ( recipient_amount.amount < BONUS_MINIMUM_AMOUNT )
					continue;

				// we need to be the recipient
				if ( recipient_amount.recipient != BONUS_ACCOUNT_RS )
					continue;


				const uint64_t tx_id = std::stoull( tx.tx_id );

				// Check whether we've dealt with this transaction already
				// (we're the only process dealing with bonuses so no need for a lock/mutex)
				Bonus bonuses;
				bonuses.tx_id( tx_id );
				if ( bonuses.present() )
					continue;

				// save bonus amount + transaction ID to DB (for idempotency check)
				// create queue of outgoing transactions based on bonus
				// commit all of above in one transaction
				DORM::Transaction db_tx_guard;

				Bonus bonus;
				bonus.tx_id( tx_id );
				bonus.amount( recipient_amount.amount );
				bonus.seen_when( tx.unix_timestamp );
				bonus.save();

				// bonus is spread out using historic share fractions
				auto shares = Share::historic_shares( latest_blockID - 1, HISTORIC_BLOCK_COUNT );
				uint64_t num_shares = shares->search();
				uint64_t feesNQT = num_shares * PAYMENT_SEND_FEE;
				uint64_t feeless_balanceNQT = recipient_amount.amount - feesNQT;

				while( auto share = shares->result() ) {
					uint64_t amount = feeless_balanceNQT * share->share_fraction();

					Reward reward;
					reward.accountID( share->accountID() );
					reward.amount( amount );
					reward.blockID( latest_blockID );
					reward.bonusID( bonus.bonusID() );
					reward.save();
				}

				// before we commit, forward the bonus through to the main pool account
				CryptoCoinTx forward_tx;
				forward_tx.sender = BONUS_ACCOUNT_RS;
				forward_tx.encoded_passphrase = BONUS_ACCOUNT_PASSPHRASE;
				forward_tx.fee_inclusive = false;
				forward_tx.fee = PAYMENT_SEND_FEE;

				forward_tx.recipient_amounts.push_back( { OUR_ACCOUNT_RS, bonus.amount() - PAYMENT_SEND_FEE } );

				try {
					if ( !burst.send_transaction(forward_tx) ) {
						std::cout << ftime() << "Couldn't forward bonus of " << burst.pretty_amount(bonus.amount()) << " to " << OUR_ACCOUNT_RS << std::endl;

						// this is somewhat unexpected - notify someone
						const time_t now = time(nullptr);

						if (now > bonus_push_cooloff_timeout) {
							bonus_push_cooloff_timeout = now + PUSH_COOLOFF_PERIOD;
							push_notification(PUSH_URL, PUSH_KEY, PUSH_RECIPIENT, "Can't forward bonus to pool account");
						}

						bonus_pay_cooloff_timeout = now + PAY_FAILURE_COOLOFF_PERIOD;
						break;
					}

					std::cout << ftime() << "Forwarded bonus of " << burst.pretty_amount(bonus.amount()) << " to " << OUR_ACCOUNT_RS << ", forward tx id " << forward_tx.tx_id << std::endl;
				} catch (const CryptoCoins::server_issue &e) {
					// something went wrong - cool-off for a while
					bonus_pay_cooloff_timeout = time(nullptr) + PAY_FAILURE_COOLOFF_PERIOD;
					break;
				}

				db_tx_guard.commit();

				std::cout << ftime() << "Bonus transaction processed [tx id " << tx.tx_id << "]: " << burst.pretty_amount(recipient_amount.amount) << std::endl;

				// (some other process will send rewards and check their confirmation)
			}
		}
	}
}

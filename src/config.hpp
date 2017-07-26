#ifndef SRC__CONFIG_HPP
#define SRC__CONFIG_HPP

#include "config_loader.hpp"

#include <string>
#include <vector>
#include <stdint.h>

extern std::vector<std::string> BURST_SERVERS;
extern int BURST_SERVER_TIMEOUT;

extern std::string POOL_NAME;
extern std::string POOL_HOSTNAME;

// 10 minutes is about 3ish blocks
extern uint64_t ACCOUNT_UPDATE_TIMEOUT;

// our NUMERIC account ID
extern uint64_t OUR_ACCOUNT_ID;
extern std::string OUR_ACCOUNT_RS; /* auto-converted into RS form */

// our account passphrase but encoded
// (temporarily decoded in memory when needed)
extern std::string OUR_ACCOUNT_PASSPHRASE;

// maximum deadline that pool accepts from a miner
// e.g. 30 days
extern uint64_t DEADLINE_MAX;

// deadlines greater than this value suggest miner has a bad plot or on wrong block
// e.g. 100 years?
extern uint64_t DEADLINE_BAD;

// how many blocks to go back to determine historic shares
extern unsigned int HISTORIC_BLOCK_COUNT;

// how many blocks to go back to estimate miner's capacity
extern unsigned int HISTORIC_CAPACITY_BLOCK_COUNT;

// number of seconds that must elapse before a miner can submit a nonce again
extern unsigned int SUBMIT_NONCE_COOLOFF;

// pool fee (e.g. 0.02 is 2%)
extern double POOL_FEE_FRACTION;

// NUMERIC pool fee account
extern uint64_t POOL_FEE_ACCOUNT_ID;
extern std::string POOL_FEE_ACCOUNT_RS;

// minimum amount before a payout is made to a miner
extern uint64_t MINIMUM_PAYOUT;
extern uint64_t MINIMUM_DEFERRED_PAYOUT;

extern double SHARE_POWER_FACTOR;

extern unsigned int CURRENT_BLOCK_REWARD_PERCENT;

// how many blocks to show on the status webpage
extern unsigned int RECENT_BLOCK_HISTORY_DEPTH;

// maximum number of blocks until a reward is paid out regardless of amount
extern unsigned int MAX_PAYOUT_BLOCK_DELAY;

// minimum number of blocks until a winning block reward is shared out
// (this is to allow for network to converge on who won the block)
extern unsigned int MIN_PAYOUT_BLOCK_DELAY;

// account details for bonus account that shares incoming payments to miners
extern uint64_t BONUS_ACCOUNT_ID;
extern std::string BONUS_ACCOUNT_RS;
extern std::string BONUS_ACCOUNT_PASSPHRASE;
extern uint64_t BONUS_MINIMUM_AMOUNT;

// details for push notifications via pushover.net
extern std::string PUSH_URL;
extern std::string PUSH_KEY;
extern std::string PUSH_RECIPIENT;

// Numeric accountIDs of banned miners
extern std::vector<uint64_t> BANNED_ACCOUNT_IDS;

#endif

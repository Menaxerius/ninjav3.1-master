#include "Account.hpp"

#include "config_loader.hpp"
#include "config.hpp"

#include <iostream>


std::vector<std::string> BURST_SERVERS;
int BURST_SERVER_TIMEOUT;

std::string POOL_NAME;
std::string POOL_HOSTNAME;

uint64_t ACCOUNT_UPDATE_TIMEOUT;

uint64_t OUR_ACCOUNT_ID;
std::string OUR_ACCOUNT_RS;
std::string OUR_ACCOUNT_PASSPHRASE;

uint64_t DEADLINE_MAX;
uint64_t DEADLINE_BAD;

unsigned int HISTORIC_BLOCK_COUNT;
unsigned int HISTORIC_CAPACITY_BLOCK_COUNT;

unsigned int SUBMIT_NONCE_COOLOFF;

double POOL_FEE_FRACTION;
uint64_t POOL_FEE_ACCOUNT_ID;
std::string POOL_FEE_ACCOUNT_RS;

uint64_t MINIMUM_PAYOUT;

double SHARE_POWER_FACTOR;
unsigned int CURRENT_BLOCK_REWARD_PERCENT;
unsigned int RECENT_BLOCK_HISTORY_DEPTH;

unsigned int MAX_PAYOUT_BLOCK_DELAY;
unsigned int MIN_PAYOUT_BLOCK_DELAY;

uint64_t BONUS_ACCOUNT_ID;
std::string BONUS_ACCOUNT_RS;
std::string BONUS_ACCOUNT_PASSPHRASE;
uint64_t BONUS_MINIMUM_AMOUNT;

std::string PUSH_URL;
std::string PUSH_KEY;
std::string PUSH_RECIPIENT;

std::vector<uint64_t> BANNED_ACCOUNT_IDS;


void more_config( const JSON &json ) {
	JSON_Array servers_json = json.get_array("burstServers");
	for( int i=0; i<servers_json.size(); ++i )
		BURST_SERVERS.push_back( servers_json.get_string(i) );

	BURST_SERVER_TIMEOUT = json.get_number("burstServerTimeout");

	POOL_NAME = json.get_string("poolName");
	POOL_HOSTNAME = json.get_string("poolHostname");

	ACCOUNT_UPDATE_TIMEOUT = json.get_number("accountUpdateTimeout");
	OUR_ACCOUNT_ID = json.get_uint64("poolNumericAccountId");
	OUR_ACCOUNT_RS = BurstCoin::accountID_to_RS_string( OUR_ACCOUNT_ID );
	OUR_ACCOUNT_PASSPHRASE = json.get_string("poolEncodedPassphrase");

	DEADLINE_MAX = json.get_uint64("poolDeadlineLimit");
	DEADLINE_BAD = json.get_uint64("poolDeadlineReallyBad");

	HISTORIC_BLOCK_COUNT = json.get_number("historicBlockCount");
	HISTORIC_CAPACITY_BLOCK_COUNT = json.get_number("capacityBlockCount");

	SUBMIT_NONCE_COOLOFF = json.get_number("submitNonceCooloff");

	POOL_FEE_FRACTION = json.get_double("poolFeePercent") / 100.0;
	POOL_FEE_ACCOUNT_ID = json.get_uint64("poolFeeNumericAccountId");
	POOL_FEE_ACCOUNT_RS = BurstCoin::accountID_to_RS_string( POOL_FEE_ACCOUNT_ID );

	MINIMUM_PAYOUT = json.get_uint64("minimumPayout") * 100000000; // config file value is in BURST, not NQT

	SHARE_POWER_FACTOR = json.get_double("sharePowerFactor");
	CURRENT_BLOCK_REWARD_PERCENT = json.get_number("currentBlockRewardPercent");

	RECENT_BLOCK_HISTORY_DEPTH = json.get_uint64("recentBlockHistoryDepth");

	MAX_PAYOUT_BLOCK_DELAY = json.get_uint64("maximumPayoutBlockDelay");
	MIN_PAYOUT_BLOCK_DELAY = json.get_uint64("minimumPayoutBlockDelay");

	BONUS_ACCOUNT_ID = json.get_uint64("bonusNumericAccountId");
	BONUS_ACCOUNT_RS = BurstCoin::accountID_to_RS_string( BONUS_ACCOUNT_ID );
	BONUS_ACCOUNT_PASSPHRASE = json.get_string("bonusEncodedPassphrase");
	BONUS_MINIMUM_AMOUNT = json.get_uint64("bonusMinimumThreshold") * 100000000; // config file value is in BURST, not NQT

	PUSH_URL = json.get_string("pushURL");
	PUSH_KEY = json.get_string("pushKey");
	PUSH_RECIPIENT = json.get_string("pushRecipient");

	if ( !json.null("bannedAccountIds") ) {
		JSON_Array accounts_json = json.get_array("bannedAccountIds");
		for(int i=0; i<accounts_json.size(); ++i)
			BANNED_ACCOUNT_IDS.push_back( accounts_json.get_uint64(i) );
	}


	// late-stage validity checks
	if ( (LISTEN_PORT < 1) || (LISTEN_PORT > 65535) ) {
		std::cerr << "listenPort needs to be between 1 and 65535" << std::endl;
		exit(2);
	}

	if ( DEADLINE_MAX == 0 ) {
		std::cerr << "poolDeadlineLimit - maximum deadline accepted (in seconds)" << std::endl;
		exit(2);
	}

	if ( DEADLINE_BAD == 0 ) {
		std::cerr << "poolDeadlineReallyBad - threshold for 'deadline REALLY BAD' message (seconds)" << std::endl;
		exit(2);
	}

	if ( DEADLINE_BAD <= DEADLINE_MAX ) {
		std::cerr << "poolDeadlineReallyBad should be way bigger than poolDeadlineLimit" << std::endl;
		exit(2);
	}

	if ( HISTORIC_BLOCK_COUNT == 0 ) {
		std::cerr << "historicBlockCount - number of previous blocks to take into account for rewards" << std::endl;
		exit(2);
	}
}

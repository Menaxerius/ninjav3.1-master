#include "XHR/getRecentBlocks.hpp"

#include "Nonce.hpp"
#include "Block.hpp"
#include "Account.hpp"

#include "BlockCache.hpp"
#include "mining_info.hpp"
#include "config.hpp"

#include "JSON.hpp"


int Handlers::XHR::getRecentBlocks::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	uint64_t latest_blockID = BlockCache::latest_blockID;

	Block blocks;
	blocks.newest_first(true);
	blocks.after_blockID( latest_blockID - RECENT_BLOCK_HISTORY_DEPTH );
	blocks.before_blockID( latest_blockID );
	blocks.search();

	JSON_Array blocks_json;

	while( auto block = blocks.result() ) {
		JSON entry;
		entry.add_uint( "block", block->blockID() );

		Account::account_info_JSON( block->generator_account_id(), entry, "generator" );

		if ( block->best_nonce_account_id() ) {
			Account::account_info_JSON( block->best_nonce_account_id(), entry, "ourBest" );

			entry.add_uint( "ourBestDeadline", block->our_best_deadline() );
		}

		entry.add_uint( "blockReward", block->block_reward() );
		entry.add_uint( "isOurBlock", block->is_our_block() );
		entry.add_uint( "deadline", block->deadline() );

		blocks_json.push_back(entry);
	}

	JSON json;
	json.add_item("blocks", blocks_json);

	resp->status_code = 200;
	resp->content = json.to_string();
	resp->add_header( "Access-Control-Allow-Origin", "*" );

	return MHD_YES;
}

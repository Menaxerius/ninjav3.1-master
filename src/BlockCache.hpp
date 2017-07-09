#ifndef SRC__BLOCKCACHE_HPP
#define SRC__BLOCKCACHE_HPP

#include "Block.hpp"

#include <mutex>
#include <unistd.h>


class BlockCache {
	private:
		static std::unique_ptr<Block>	latest_block;
		static std::unique_ptr<Block>	previous_block;
		static std::mutex				block_mutex;
		static std::string				mining_info;
		static std::string				latest_generation_signature;

	public:
		static uint64_t					latest_blockID;

		static std::string get_mining_info();
		static bool update_latest_block(const uint64_t blockID, const uint64_t base_target, const std::string &generation_signature);

		static void inc_rejected_nonces();
		static void update_best_nonce( const std::unique_ptr<Nonce> &nonce );

		static std::unique_ptr<Block> clone_latest_block();
		static std::unique_ptr<Block> clone_previous_block();

		static uint64_t rejected_nonce_count();
};


#endif

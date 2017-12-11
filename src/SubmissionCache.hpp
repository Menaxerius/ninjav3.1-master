#ifndef SRC__SUBMISSIONCACHE_HPP
#define SRC__SUBMISSIONCACHE_HPP

#include "Nonce.hpp"
#include "Share.hpp"

#include <list>
#include <mutex>

class SubmissionCache {
	private:
		typedef std::list< std::unique_ptr<Nonce> > nonces_t;
		static nonces_t					nonces;
		static std::mutex				nonces_mutex;

		static std::unique_ptr<Nonce>	first_submitted_nonce;
		static std::unique_ptr<Nonce>	last_submitted_nonce;

		
		typedef std::list< std::unique_ptr<Share> > shares_t;
		static shares_t					shares;
		static std::mutex				shares_mutex;

		static const std::unique_ptr<Nonce> save_nonce(Nonce &new_nonce);
		static void recalculate_shares();

	public:
		static void save_and_rank(Nonce &new_nonce);
		static std::list< std::unique_ptr<Share> > clone_shares();

		static std::unique_ptr<Nonce> clone_first_nonce();	// "first" as in first submitted by miner after new block
		static std::unique_ptr<Nonce> clone_last_nonce();	// "last" as in most recently submitted by miner
		static std::unique_ptr<Nonce> clone_best_nonce();	// "best" as in the nonce with smallest/best deadline
		static std::unique_ptr<Nonce> clone_worst_nonce();	// "worst" as in the nonce with largest/worst deadline

		static uint64_t nonce_count();
		static uint64_t share_count();

		static void new_block_reset(const bool try_loading_from_DB);
};

#endif

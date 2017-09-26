#include "SubmissionCache.hpp"
#include "BlockCache.hpp"
#include "config.hpp"

#include <cmath>


SubmissionCache::nonces_t		SubmissionCache::nonces;
std::mutex						SubmissionCache::nonces_mutex;

std::unique_ptr<Nonce>			SubmissionCache::first_submitted_nonce;
std::unique_ptr<Nonce>			SubmissionCache::last_submitted_nonce;

SubmissionCache::shares_t		SubmissionCache::shares;
std::mutex						SubmissionCache::shares_mutex;


const std::unique_ptr<Nonce> SubmissionCache::save_nonce(Nonce &new_nonce) {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	// quick exit if this is a duplicate
	for( const auto &nonce : nonces )
		if (nonce->accountID() == new_nonce.accountID() && nonce->nonce() == new_nonce.nonce() )
			return nonces.front()->clone();

	// it's new - save it to DB
	new_nonce.save();

	// is it the first?
	if ( !first_submitted_nonce ) {
		auto nonce_clone = new_nonce.clone();
		first_submitted_nonce.swap(nonce_clone);
	}

	auto last_nonce_clone = new_nonce.clone();
	last_submitted_nonce.swap(last_nonce_clone);

	// sorted insert into nonces for this block
	auto nonces_it = nonces.cbegin();
	while( nonces_it != nonces.cend() && (*nonces_it)->deadline() <= new_nonce.deadline() )
		++nonces_it;

	auto nonce_clone = new_nonce.clone();
	nonces.emplace( nonces_it, std::move(nonce_clone) );

	return nonces.front()->clone();
}


void SubmissionCache::recalculate_shares() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);
	std::lock_guard<std::mutex> shares_guard(shares_mutex);

	// We process each account's best nonce ignoring other nonces from the same account.
	// As nonces are sorted, we can simply keep a record of which accounts we've come across.
	std::map<uint64_t, const Nonce *> nonces_by_accountID;

	double total_shares = 0.0;
	std::list< std::pair<uint64_t,double> > shares_by_accountID;

	for( const auto &nonce : nonces ) {
		auto insert_result = nonces_by_accountID.insert( std::make_pair( nonce->accountID(), nonce.get() ) );
		if (insert_result.second == false)
			continue;	// key (accountID) already existed

		// it's actually possible (although rare) for deadline to be zero!
		const uint64_t deadline = nonce->deadline() + 1;

		const double share = 1.0 / pow( static_cast<double>(deadline), SHARE_POWER_FACTOR );
		shares_by_accountID.push_back( std::make_pair( nonce->accountID(), share ) );

		total_shares += share;
	}

	shares.clear();
	// no need to wipe old shares in database
	// as existing shares only get insert/updated, never deleted

	for( const auto &share_pair : shares_by_accountID ) {
		const Nonce *nonce = nonces_by_accountID[share_pair.first];
		const double share_fraction = share_pair.second / total_shares;

		auto share = std::make_unique<Share>();
		share->blockID( nonce->blockID() );
		share->accountID( nonce->accountID() );
		share->share_fraction( share_fraction );
		share->deadline( nonce->deadline() );
		share->deadline_string( nonce->deadline_string() );
		share->miner( nonce->miner() );
		share->save();

		shares.push_back( std::move(share) );
	}
}


void SubmissionCache::save_and_rank(Nonce &new_nonce) {
	const std::unique_ptr<Nonce> &best_nonce = save_nonce(new_nonce);

	// we can update BlockCache's best nonce info now
	BlockCache::update_best_nonce( best_nonce );

	// recalculate shares!
	// recalculate_shares();
}


std::list< std::unique_ptr<Share> > SubmissionCache::clone_shares() {
	std::lock_guard<std::mutex> shares_guard(shares_mutex);

	// return snapshot of shares
	std::list< std::unique_ptr<Share> > cloned_list;

	for( const auto &share : shares ) {
		auto share_clone = share->clone();
		cloned_list.push_back( std::move(share_clone) );
	}

	return cloned_list;
}


std::unique_ptr<Nonce> SubmissionCache::clone_first_nonce() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	if (!first_submitted_nonce)
		return std::unique_ptr<Nonce>();

	return first_submitted_nonce->clone();
}


std::unique_ptr<Nonce> SubmissionCache::clone_last_nonce() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	if (!last_submitted_nonce)
		return std::unique_ptr<Nonce>();

	return last_submitted_nonce->clone();
}


std::unique_ptr<Nonce> SubmissionCache::clone_best_nonce() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	if ( nonces.empty() )
		return std::unique_ptr<Nonce>();

	return nonces.front()->clone();
}


std::unique_ptr<Nonce> SubmissionCache::clone_worst_nonce() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	if ( nonces.empty() )
		return std::unique_ptr<Nonce>();

	return nonces.back()->clone();
}


uint64_t SubmissionCache::nonce_count() {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);

	return nonces.size();
}


uint64_t SubmissionCache::share_count() {
	std::lock_guard<std::mutex> shares_guard(shares_mutex);

	return shares.size();
}


void SubmissionCache::new_block_reset(const bool try_loading_from_DB) {
	std::lock_guard<std::mutex>	nonces_guard(nonces_mutex);
	nonces.clear();
	first_submitted_nonce.reset();
	last_submitted_nonce.reset();

	if (try_loading_from_DB) {
		Nonce DB_nonces;
		DB_nonces.blockID(BlockCache::latest_blockID);
		DB_nonces.best_first(true);
		DB_nonces.search();

		while( auto DB_nonce = DB_nonces.result() )
			nonces.push_back( std::move(DB_nonce) );
	}

	std::lock_guard<std::mutex> shares_guard(shares_mutex);
	shares.clear();

	if (try_loading_from_DB) {
		Share DB_shares;
		DB_shares.blockID(BlockCache::latest_blockID);
		DB_shares.biggest_first(true);
		DB_shares.search();

		while( auto DB_share = DB_shares.result() )
			shares.push_back( std::move(DB_share) );
	}
}

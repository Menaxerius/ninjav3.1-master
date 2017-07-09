#include "Nonce.hpp"

#include "Account.hpp"
#include "Block.hpp"

#include "mining_info.hpp"

#include "DORM/sql/sqlLt.hpp"

extern "C" {
	#include "shabal.h"
}

#ifdef __FreeBSD__
	#include <sys/endian.h>
#endif

#include <cstring>


void Nonce::search_prep( DORM::Query &query ) const {
	if (newest_first)
		query.order_by = "submitted_when ASC";

	if (oldest_first)
		query.order_by = "submitted_when DESC";

	if (best_first)
		query.order_by = "deadline ASC";

	if (worst_first)
		query.order_by = "deadline DESC";

	if (before_blockID)
		query.and_where( DORM::sqlLt<uint64_t>( "blockID", before_blockID() ) );
}


uint8_t *Nonce::plot_nonce( uint64_t account_id, uint64_t nonce ) {
	uint8_t final[HASH_SIZE];

	#ifndef DEV_MODE
		// aligned to 256 to support future AVX shabal code
		// right now alignment to 16 would probably be enough
		uint8_t *gendata = (uint8_t *)aligned_alloc(256, 16 + PLOT_SIZE);
		if (gendata == NULL) {
			perror("aligned_alloc");
			exit(2);
		}
	#else
		// for leak checking we have to use malloc()
		uint8_t *gendata = (uint8_t *)malloc(16 + PLOT_SIZE);
		if (gendata == NULL) {
			perror("malloc");
			exit(2);
		}
	#endif

	// copy account address to plot end
	uint64_t account_id_swapped = htobe64(account_id);
	memcpy(gendata + PLOT_SIZE, (uint8_t *)&account_id_swapped, 8);

	uint8_t *xv = (uint8_t *)&nonce;

	gendata[PLOT_SIZE+8] = xv[7]; gendata[PLOT_SIZE+9] = xv[6]; gendata[PLOT_SIZE+10] = xv[5]; gendata[PLOT_SIZE+11] = xv[4];
	gendata[PLOT_SIZE+12] = xv[3]; gendata[PLOT_SIZE+13] = xv[2]; gendata[PLOT_SIZE+14] = xv[1]; gendata[PLOT_SIZE+15] = xv[0];

	shabal_context fresh_sc;
	shabal_init(&fresh_sc, 256);

	shabal_context tmp_sc;

	uint32_t len = 16;

	for(int i = PLOT_SIZE; i > 0; i -= HASH_SIZE) {
		memcpy(&tmp_sc, &fresh_sc, sizeof(shabal_context)); // in lieu of: shabal_init(&tmp_sc, 256);

		shabal(&tmp_sc, &gendata[i], len);
		shabal_close(&tmp_sc, 0, 0, &gendata[i - HASH_SIZE]);

		if (len < HASH_CAP) {
			len += HASH_SIZE;
			if (len > HASH_CAP)
				len = HASH_CAP;
		}
	}

	memcpy(&tmp_sc, &fresh_sc, sizeof(shabal_context)); // in lieu of: shabal_init(&tmp_sc, 256);
	shabal(&tmp_sc, gendata, 16 + PLOT_SIZE);
	shabal_close(&tmp_sc, 0, 0, final);

	// XOR with final
	for(int i = 0; i < PLOT_SIZE; i ++)
		gendata[i] ^= (final[i % 32]);

	return gendata;
}


uint64_t Nonce::calculate_deadline( uint64_t account_id, uint64_t nonce, uint64_t blockID, uint32_t scoop, uint64_t base_target, const std::string &gen_sig_str ) {
	uint8_t gen_sig[32];
	Block::unpack_generation_signature( gen_sig_str, gen_sig );

	uint8_t *gendata = Nonce::plot_nonce( account_id, nonce );

	uint8_t final[HASH_SIZE];

	shabal_context deadline_sc;
	shabal_init(&deadline_sc, 256);
	shabal(&deadline_sc, gen_sig, HASH_SIZE);

	shabal(&deadline_sc, gendata + (scoop * SCOOP_SIZE), SCOOP_SIZE);
	shabal_close(&deadline_sc, 0, 0, final);

	uint64_t target_result = *(uint64_t *)final;

	free(gendata);

	return target_result / base_target;
}


std::string Nonce::deadline_to_string( uint64_t deadline ) {
	static const std::string units[] = { "year", "month", "day", "hour", "min", "sec" };
	static const uint64_t unit_multipliers[] = { 365*24*60*60, 30*24*60*60, 24*60*60, 60*60, 60, 1 };

	if (deadline < 2)
		return std::to_string(deadline) + " " + units[5];

	std::string deadline_string;
	for(int i=0; i<6; i++) {
		if (deadline > unit_multipliers[i]) {
			if ( !deadline_string.empty() )
				deadline_string += ", ";

			uint64_t n_units = deadline / unit_multipliers[i];
			deadline = deadline % unit_multipliers[i];
			deadline_string += std::to_string(n_units) + " " + units[i];

			if (n_units > 1)
				deadline_string += "s";
		}
	}

	return deadline_string;
}

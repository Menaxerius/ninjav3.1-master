#include "mining_info.hpp"
//#include "config.hpp"

#include "BurstCoin.hpp"
#include "JSON.hpp"

extern "C" {
	#include "shabal.h"
}

#ifdef __FreeBSD__
	#include <sys/endian.h>
#endif

#include <cstring>
#include <iostream>
#include <sstream>


class Block {
	private:
		typedef uint8_t gen_sig_array_t[32];

	public:
		static void unpack_generation_signature( const std::string &gen_sig_str, gen_sig_array_t &gen_sig_array );
		static uint32_t calculate_scoop( const uint64_t scoop_blockID, const std::string &scoop_generation_signature );
};


void Block::unpack_generation_signature( const std::string &gen_sig_str, gen_sig_array_t &gen_sig_array ) {
	for(int i=0; i<32; i++)
		gen_sig_array[i] = std::stoul( gen_sig_str.substr( i<<1, 2).c_str(), nullptr, 16 );
}


uint32_t Block::calculate_scoop( const uint64_t scoop_blockID, const std::string &scoop_generation_signature ) {
	// shabal256 hash of gen_sig (in binary form) and then block number (uint64_t big-endian)
	shabal_context sc;
	gen_sig_array_t binary_generation_signature;
	unpack_generation_signature( scoop_generation_signature, binary_generation_signature );

	shabal_init(&sc, 256);
	shabal(&sc, binary_generation_signature, 32);

	const uint64_t block_swapped = htobe64( scoop_blockID );
	shabal(&sc, &block_swapped, sizeof(block_swapped));

	uint8_t new_generation_signature[32];
	shabal_close(&sc, 0, 0, new_generation_signature);

	// finally we get to determine scoop number
	return ((new_generation_signature[30] & 0x0F) << 8) | new_generation_signature[31];
}


class Nonce {
	public:
		static uint8_t *plot_nonce( uint64_t account_id, uint64_t nonce );
		static uint64_t calculate_deadline( uint64_t account_id, uint64_t nonce, uint64_t blockID, uint32_t scoop, uint64_t base_target, const std::string &gen_sig_str );
		static std::string deadline_to_string( uint64_t deadline );
};


uint8_t *Nonce::plot_nonce( uint64_t account_id, uint64_t nonce ) {
	uint8_t final[HASH_SIZE];

	#ifndef DEV_MODE
		// aligned to 256 to support future AVX shabal code
		// right now alignment to 16 would probably be enough
		uint8_t *gendata = (uint8_t *)aligned_alloc(256, 16 + PLOT_SIZE);
		if (gendata == nullptr) {
			perror("aligned_alloc");
			exit(2);
		}
	#else
		// for leak checking we have to use malloc()
		uint8_t *gendata = (uint8_t *)malloc(16 + PLOT_SIZE);
		if (gendata == nullptr) {
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

// {"generationSignature":"80e86602ce1241fd0b7909f5e9e905e1c68e18cba9f1ac8edeca47060a5d3623","baseTarget":"872511","requestProcessingTime":0,"height":"361572"}
// {"generationSignature":"918b16185bb316dd6d179e698fe57d99410b9a5fd10806b76d60cb0b8bf98db1","baseTarget":"994332","requestProcessingTime":0,"height":"361584"}


int main(int argc, char *argv[]) {
	if (argc != 5) {
		std::cerr << "usage: " << argv[0] << " burst-server accountID nonce blockID" << std::endl;
		return 1;
	}

	BurstCoin burst( argv[1] );
	const uint64_t accountID = std::stoull( argv[2] );
	const uint64_t nonce = std::stoull( argv[3] );
	const uint64_t blockID = std::stoull( argv[4] );

	std::string block_info = burst.get_block( blockID - 1 );
	JSON block_json(block_info);

	std::string gen_sig_str = block_json.get_string("generationSignature");
	uint64_t base_target = block_json.get_uint64("baseTarget");
	uint64_t generatorID = htobe64( block_json.get_uint64("generator") );

	std::cout << "Previous block's generation-signature: " << gen_sig_str << ", base-target: " << base_target << ", generator: " << generatorID << std::endl;

	uint8_t gen_sig_array[32];
	Block::unpack_generation_signature( gen_sig_str, gen_sig_array );

	shabal_context new_gen_sig_sc;
	uint8_t final[HASH_SIZE];

	shabal_init(&new_gen_sig_sc, 256);
	shabal(&new_gen_sig_sc, gen_sig_array, HASH_SIZE);
	shabal(&new_gen_sig_sc, &generatorID, 8);
	shabal_close(&new_gen_sig_sc, 0, 0, final);

	std::string new_gen_sig_str;
	for(int i=0; i<32; i++) {
		std::stringstream ss;
		ss << std::hex;
		ss.fill('0');
		ss.width(2);
		ss << static_cast<unsigned int>(final[i]);
		new_gen_sig_str += ss.str();
	}

	uint32_t scoop = Block::calculate_scoop( blockID, new_gen_sig_str );

	uint64_t deadline = Nonce::calculate_deadline( accountID, nonce, blockID, scoop, base_target, new_gen_sig_str );

	std::cout << "Mining generation-signature: " << new_gen_sig_str << ", base-target: " << base_target << ", scoop: " << scoop << "\n";
	std::cout << "AccountID: " << accountID << ", nonce: " << nonce << ", blockID: " << blockID << "\n";
	std::cout << "Deadline: " << deadline << ", " << Nonce::deadline_to_string(deadline) << std::endl;

	return 0;
}

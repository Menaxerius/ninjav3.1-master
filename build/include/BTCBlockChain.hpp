#ifndef INCLUDE__BTCBLOCKCHAIN_HPP
#define INCLUDE__BTCBLOCKCHAIN_HPP

#include <stdint.h>
#include <string>
#include <stdlib.h>

typedef uint64_t compactInt;

struct BTCTxOutpoint {
	char					hash[32];
	uint32_t				index;
};


struct BTCTxIn {
	struct BTCTxOutpoint	previous_output;
	compactInt				script_bytes;
	uint8_t					*script = NULL;
	uint32_t				sequence;

	~BTCTxIn() {
		if (script)
			free(script);
	}
};


struct BTCTxOut {
	uint64_t				value;
	compactInt				pk_script_bytes;
	uint8_t					*pk_script = NULL;

	~BTCTxOut() {
		if (pk_script)
			free(pk_script);
	}
};


struct BTCCoinBase {
	char					hash[32];
	uint32_t				index;
	compactInt				script_bytes;
	uint32_t				height;
	uint8_t					*script = NULL;
	uint32_t				coinbase_sequence;

	~BTCCoinBase() {
		if (script)
			free(script);
	}
};


struct BTCTx {
	uint32_t				version;
	compactInt				tx_in_count;
	struct BTCTxIn			*tx_ins = NULL;
	compactInt				tx_out_count;
	struct BTCTxOut			*tx_outs = NULL;
	uint32_t				locktime;

	~BTCTx() {
		if (tx_ins)
			delete[] tx_ins;
		if (tx_outs)
			delete[] tx_outs;
	}
};


struct BTCBlock {
	uint32_t				magic;
	uint32_t				length;
	uint32_t				version;
	char					previous_hash[32];
	char					merkle_root_hash[32];
	uint32_t				time;
	uint32_t				nBits;
	uint32_t				nonce;
	compactInt				txn_count;
	struct BTCTx			*txns = NULL;
	uint64_t				height;	// not actually on disc

	~BTCBlock() {
		if (txns)
			delete[] txns;
	}
};


class BTCBlockChain {
	private:
		std::string		datadirpath;
		std::string		statefilepath;
		struct {
			uint64_t		file;
			uint64_t		block_offset;
			uint64_t		tx_offset;
			uint64_t		block;
			uint64_t		tx_index;
		} state;
		int				block_fd;
		uint64_t		tx_count;
		uint64_t		file_length;
		
		template<typename T>
		void read_t(int fd, T &dest);

		void read_compactInt(int fd, compactInt &dest);
		void read_n(int fd, void *dest, uint64_t len);

	public:
		BTCBlockChain( std::string datadir );
		BTCBlockChain( std::string datadir, std::string statefile );
		~BTCBlockChain();

		struct BTCBlock *get_next_block();
		struct BTCTx *get_next_tx();
};


#endif

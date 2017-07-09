#include "BTCBlockChain.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

#include <time.h>
#include <string.h>


union compact_int {
	uint8_t		value8;
	uint16_t	value16;
	uint32_t	value32;
	uint64_t	value64;
};


const uint32_t BTC_BLOCK_MAGIC = 0xd9b4bef9; // little-endian


template<typename T>
void BTCBlockChain::read_t(int fd, T &dest) {
	int res = read( fd, (void *)&dest, sizeof(dest) );
	if (res == -1) {
		perror("Can't read from block file");
		exit(2);
	} else if ( res < sizeof(dest) ) {
		perror("Short read from block file");
		exit(2);
	}
} 


void BTCBlockChain::read_compactInt(int fd, compactInt &dest) {
	union compact_int ci;
	read_t<uint8_t>(fd, ci.value8);
	
	if (ci.value8 <= 252) {
		dest = ci.value8;
		return;
	}
	
	if (ci.value8 == 0xfd) {
		read_t<uint16_t>(fd, ci.value16);
		dest = ci.value16;
	} else if (ci.value8 == 0xfe) {
		read_t<uint32_t>(fd, ci.value32);
		dest = ci.value32;
	} else if (ci.value8 == 0xff) {
		read_t<uint64_t>(fd, ci.value64);
		dest = ci.value64;
	} else {
		// bad!
		std::cerr << "Failed to parse compactInt" << std::endl;
		exit(2);
	}  
}


void BTCBlockChain::read_n(int fd, void *dest, uint64_t len) {
	int res = read(fd, dest, len);
	if (res == -1) {
		perror("Can't read from block file");
		exit(2);
	} else if (res < len) {
		perror("Short read from block file");
		exit(2);
	}
}


BTCBlockChain::BTCBlockChain( std::string datadir ) {
	DIR *dir = opendir( datadir.c_str() );
	if ( !dir ) {
		std::cerr << "Can't open datadir '" << datadir << "': ";
		perror(NULL);
		exit(2);
	}
	closedir(dir);

	// also check for at least the first file...
	std::string check_file = datadir + "/blk00000.dat";
	int fd = open( check_file.c_str(), O_RDONLY );
	if (fd == -1) {
		perror("Can't open first block file");
		exit(2);
	}
	close(fd);
	
	datadirpath = datadir;
	
	state = { 00000, 0, 0, 0, 0 };

	block_fd = -1;
}


BTCBlockChain::BTCBlockChain( std::string datadir, std::string statefile ) : BTCBlockChain(datadir) {
	int fd = open( statefile.c_str(), O_RDWR | O_CREAT, 0664 );
	if (fd == -1) {
		perror("Can't open/create state file");
		exit(2);
	}
	
	int nread = read(fd, &state, sizeof(state));
	if (nread == -1) {
		perror("Can't read state");
		exit(2);
	} else if ( nread > 0 && nread < sizeof(state) ) {
		std::cerr << "State file looks wrong" << std::endl;
		exit(2);
	}
	close(fd);
	
	statefilepath = statefile;
}


BTCBlockChain::~BTCBlockChain() {
	if (block_fd != -1) {
		close(block_fd);
		block_fd = -1;
	}

	if ( !statefilepath.empty() ) {
		int fd = open( statefilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664 );
		if (fd == -1) {
			perror("Can't open/create state file");
			exit(2);
		}

		int nwrite = write(fd, &state, sizeof(state));
		if (nwrite == -1) {
			perror("Can't write state");
			exit(2);
		} else if ( nwrite != sizeof(state) ) {
			std::cerr << "Couldn't save all of state?" << std::endl;
			exit(2);
		}
		close(fd);
	}
}


struct BTCBlock *BTCBlockChain::get_next_block() {
	// time move onto next block file?
	if (block_fd != -1 && state.block_offset >= file_length) {
		close(block_fd);
		block_fd = -1;

		state.file++;
		state.block_offset = 0;
		state.tx_offset = 0;
	}

	if (block_fd == -1) {
		std::stringstream ss;
		ss.fill('0');
		ss.width(5);
		ss << state.file;

		std::string blockpath = datadirpath + "/blk" + ss.str() + ".dat";

		int fd = open( blockpath.c_str(), O_RDONLY );
		if (fd == -1) {
			std::cerr << "Can't open block file '" << blockpath << "': ";
			perror(NULL);
			exit(2);
		}

		block_fd = fd;

		struct stat sb;
		int res = fstat( block_fd, &sb );
		if (res == -1) {
			perror("Can't stat block file");
			exit(2);
		}

		file_length = sb.st_size;
	}
	
	// load up block info
	int res = lseek(block_fd, state.block_offset, SEEK_SET);
	if (res == -1) {
		perror("Can't seek to current block");
		exit(2);
	}
	
	struct BTCBlock block;
	
	read_t<uint32_t>(block_fd, block.magic);
	if (block.magic != BTC_BLOCK_MAGIC)
		return NULL;
		
	read_t<uint32_t>(block_fd, block.length);
	read_t<uint32_t>(block_fd, block.version);
	read_t<char[32]>(block_fd, block.previous_hash);
	read_t<char[32]>(block_fd, block.merkle_root_hash);
	read_t<uint32_t>(block_fd, block.time);
	read_t<uint32_t>(block_fd, block.nBits);
	read_t<uint32_t>(block_fd, block.nonce);
	read_compactInt(block_fd, block.txn_count);
	block.txns = NULL;
	block.height = state.block;
	tx_count = block.txn_count;
	
	// if we've no overriding tx_offset (i.e. it's 0)
	// then set offset to current position (i.e. start of first tx)
	if (!state.tx_offset)
		state.tx_offset = lseek(block_fd, 0, SEEK_CUR) - state.block_offset;
	
	struct BTCBlock *block_copy = new BTCBlock;
	if (block_copy == NULL) {
		perror("Can't create new block info");
		exit(2);
	}
	
	memcpy( block_copy, &block, sizeof(block) );
	return block_copy;
}


struct BTCTx *BTCBlockChain::get_next_tx() {
	if (block_fd == -1) {
		std::cerr << "get_next_tx() called before get_next_block()" << std::endl;
		exit(2);
	}
	
	// no more tx in this block?
	if (state.tx_index >= tx_count) {
		state.block++;
		state.block_offset = lseek(block_fd, 0, SEEK_CUR);
		state.tx_index = 0;
		state.tx_offset = 0;
		return NULL;
	}
	
	int res = lseek(block_fd, state.block_offset + state.tx_offset, SEEK_SET);
	if (res == -1) {
		perror("Can't seek to current tx");
		exit(2);
	}
	
	struct BTCTx *tx = new struct BTCTx;
	if (tx == NULL) {
		perror("Can't create new tx info");
		exit(2);
	}
	
	read_t<uint32_t>(block_fd, tx->version);
	
	read_compactInt(block_fd, tx->tx_in_count);
	tx->tx_ins = new struct BTCTxIn[ tx->tx_in_count ];

	for(uint64_t i=0; i<tx->tx_in_count; i++) {
		struct BTCTxIn *tx_in = &tx->tx_ins[i];
		
		read_t<char[32]>(block_fd, tx_in->previous_output.hash);
		read_t<uint32_t>(block_fd, tx_in->previous_output.index);
		
		read_compactInt(block_fd, tx_in->script_bytes);
		
		tx_in->script = (uint8_t *)malloc( tx_in->script_bytes );
		read_n(block_fd, tx_in->script, tx_in->script_bytes);
		
		read_t<uint32_t>(block_fd, tx_in->sequence);
	}
		
	read_compactInt(block_fd, tx->tx_out_count);
	tx->tx_outs = new struct BTCTxOut[ tx->tx_out_count ];

	for(uint64_t i=0; i<tx->tx_out_count; i++) {
		struct BTCTxOut *tx_out = &tx->tx_outs[i];
		
		read_t<uint64_t>(block_fd, tx_out->value);
		read_compactInt(block_fd, tx_out->pk_script_bytes);
		
		tx_out->pk_script = (uint8_t *)malloc( tx_out->pk_script_bytes );
		read_n(block_fd, tx_out->pk_script, tx_out->pk_script_bytes);
	}
	
	read_t<uint32_t>(block_fd, tx->locktime);
	
	// save offset
	state.tx_offset = lseek(block_fd, 0, SEEK_CUR) - state.block_offset;
	state.tx_index++;
	
	return tx;	
}

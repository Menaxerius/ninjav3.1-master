#include "Base58.cpp"
#include "CryptoCoins.hpp"
#include "BitCoin.cpp"

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <openssl/sha.h>


#include <iostream>


int main() {
	uint8_t key[1+32+4];
	key[0] = 0x80;
	arc4random_buf( &key[1], 32 );
	
	SHA256_CTX context;
	
	// double SHA256
	uint8_t sha_buf1[32];
	SHA256_Init(&context);
	SHA256_Update(&context, key, 33);
	SHA256_Final(sha_buf1, &context);
	
	uint8_t sha_buf2[65];
	SHA256_Init(&context);
	SHA256_Update(&context, sha_buf1, 32);
	SHA256_Final(sha_buf2, &context);
	
	// first 4 bytes of double SHA256 output go to &buffer[33]..
	memcpy( &key[33], sha_buf2, 4 );
	
	
	char *out;
	base58_encode( &out, key, 1+32+4);

	std::cout << "[btc-keygen] base58 of random data: " << out << std::endl;
	
	BitCoin bc("localhost");
	CryptoCoinKeyPair kp = bc.generate_keypair();
	
	std::cout << "[btc-keygen] " << kp.priv_key << "  -  " << kp.pub_address << std::endl;
}


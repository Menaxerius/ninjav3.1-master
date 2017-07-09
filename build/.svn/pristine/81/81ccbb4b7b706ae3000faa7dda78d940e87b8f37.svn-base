#include "CryptoCoins.hpp"
#include "BurstCoin.cpp"

#include <string>

#include <iostream>


int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "usage: test-burst-keygen BURST-SERVER" << std::endl;
		exit(1);
	}

	std::string server( argv[1] );
	BurstCoin bc( server );
	CryptoCoinKeyPair kp = bc.generate_keypair();
	
	std::cout << "[burst-keygen] " << kp.priv_key << "  -  " << kp.pub_address << std::endl;
}


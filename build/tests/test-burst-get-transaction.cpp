#include "CryptoCoins.hpp"
#include "BurstCoin.cpp"

#include <string>
#include <iostream>


int main(int argc, char *argv[]) {
	if (argc < 3) {
		std::cerr << "usage: test-burst-get-transaction BURST-SERVER transactionID" << std::endl;
		exit(1);
	}

	std::string server( argv[1] );
	BurstCoin bc( server );

	CryptoCoinTx tx;
	tx.tx_id = argv[2];

	try {
		tx = bc.get_transaction(tx);
	} catch (const CryptoCoins::server_issue &e) {
		std::cout << "Server issue" << std::endl;

	} catch (const CryptoCoins::unknown_transaction &e) {
		std::cout << "Unknown transaction" << std::endl;
	}
}


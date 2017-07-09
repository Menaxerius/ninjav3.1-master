#include "CryptoCoins.hpp"

#include <sstream>
#include <stdexcept>


// --- CRYPTOCOIN --- //


int CryptoCoin::get_confirmations( CryptoCoinTx info_tx ) {
	try {
		CryptoCoinTx tx = get_transaction( info_tx );
		return tx.confirmations;
	} catch (const CryptoCoins::unknown_transaction &e) {
		// -1 means "unknown transaction" to caller
		return -1;
	}
}


std::string CryptoCoin::_pretty_amount( uint64_t amount, uint64_t amount_divisor, std::string currency_suffix ) {
	std::ostringstream ss;
	ss.precision(10);
	ss << std::fixed << static_cast<double>(amount) / static_cast<double>(amount_divisor);
	std::string result = ss.str();

	if (result != "0") {
		// strip trailing 0s
		while ( result.back() == '0' )
			result.pop_back();

		// can't leave a trailing period either
		if ( result.back() == '.' )
			result.pop_back();
	}

	return result + " " + currency_suffix;
}


// --- CRYPTOCOINS --- //

CryptoCoins::~CryptoCoins() {
	for(auto coin : coin_servers)
		delete coin;
}


void CryptoCoins::add_coin_server( CryptoCoin *coin ) {
	coin_servers.push_back( coin );
}


std::vector<CryptoCoinTx> CryptoCoins::get_recent_transactions( std::string currency, std::string address, time_t unix_time, bool include_unconfirmed ) {
	for( auto coin : coin_servers )
		if ( coin->currency == currency )
			return coin->get_recent_transactions( address, unix_time, include_unconfirmed );
	
	throw std::runtime_error("Unknown currency: " + currency);
}


CryptoCoinTx CryptoCoins::get_transaction( CryptoCoinTx tx ) {
	for( auto coin : coin_servers )
		if ( coin->currency == tx.currency )
			return coin->get_transaction( tx );
			
	throw std::runtime_error("Unknown currency: " + tx.currency);
}


bool CryptoCoins::send_transaction( CryptoCoinTx &tx ) {
	for( auto coin : coin_servers )
		if ( coin->currency == tx.currency )
			return coin->send_transaction( tx );

	throw std::runtime_error("Unknown currency: " + tx.currency);
}


int CryptoCoins::get_confirmations( CryptoCoinTx tx ) {
	for( auto coin : coin_servers )
		if ( coin->currency == tx.currency )
			return coin->get_confirmations( tx );

	throw std::runtime_error("Unknown currency: " + tx.currency);
}


CryptoCoinKeyPair CryptoCoins::generate_keypair( std::string currency ) {
	for( auto coin : coin_servers )
		if ( coin->currency == currency )
			return coin->generate_keypair();

	throw std::runtime_error("Unknown currency: " + currency);
}


std::string CryptoCoins::pretty_amount( std::string currency, uint64_t amount ) {
	for( auto coin : coin_servers )
		if ( coin->currency == currency )
			return coin->pretty_amount( amount );

	return "";
}

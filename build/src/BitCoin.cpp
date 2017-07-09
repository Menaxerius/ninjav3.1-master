#include "BitCoin.hpp"

#include "JSON.hpp"
#include "fetch.hpp"
#include "Base58.hpp"
#include "ftime.hpp"
#include "SHA.hpp"

#include <iostream>
#include <sstream>
#include <cmath>

#include <string.h>
#include <sys/types.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/obj_mac.h>


const std::string BitCoin::currency_suffix = "BTC";
std::map<std::string, uint64_t> BitCoin::block_height_by_hash;

		
BitCoin::BitCoin( std::string srv ) {
	// does server have auth data in?
	size_t at_pos = srv.find('@');
	if (at_pos != std::string::npos) {
		server = srv.substr(at_pos + 1);
		server_auth = srv.substr(0, at_pos);
	} else {
		server = srv;
	}

	currency = "BTC";
}


JSON BitCoin::server_RPC( std::string method, JSON_Array *params ) {
	JSON json;
	json.add_string("jsonrpc", "1.0");
	json.add_string("method", method);

	if (params)
		json.add_item("params", *params);

	#ifdef DEBUG_BITCOIN
		std::cout << "[To Bitcoin server]: " << json.to_string() << std::endl;
	#endif

	std::string response = fetch( "http://" + server, json.to_string(), "application/json", server_auth );

	#ifdef DEBUG_BITCOIN
		std::cout << "[From Bitcoin server]: " << response << std::endl;
	#endif

	if ( response.empty() )
		throw CryptoCoins::server_issue();

	JSON output = JSON(response);

	// check for "warming up" error code
	if ( !output.null("error") && output.get_item("error").get_number("code") == -28 )
		throw CryptoCoins::server_issue();

	return output;
}


std::string BitCoin::get_vout_address( JSON vout_json ) {
	if ( vout_json.exists("scriptPubKey") ) {
		JSON pub_script_json = vout_json.get_item("scriptPubKey");
		
		if ( pub_script_json.exists("addresses") ) {
			JSON_Array addresses_json = pub_script_json.get_array("addresses");

			// we only need first
			if (addresses_json.size() > 0)
				return addresses_json.get_string(0);
		}
	}
	
	// no luck
	return "";
}


JSON BitCoin::get_raw_transaction( std::string tx_hash ) {
	JSON_Array params;
	params.push_back( tx_hash );
	params.push_back( 1 );		// should be a boolean really

	return server_RPC("getrawtransaction", &params);
}


uint64_t BitCoin::get_block_height( std::string block_hash ) {
	// check cache first
	const auto &map_it = block_height_by_hash.find( block_hash );
	if ( map_it != block_height_by_hash.end() )
		return map_it->second;

	// cache miss - go look it up
	JSON_Array params;
	params.push_back( block_hash );
	params.push_back( true );

	// (could add try-catch block if we need to return a height of 0)
	JSON response_json = server_RPC("getblockheader", &params);
	JSON result = response_json.get_item("result");

	uint64_t height = result.get_number("height");

	// save into cache
	block_height_by_hash[ block_hash ] = height;

	return height;
}


bool BitCoin::unlock_wallet( const std::string encoded_passphrase ) {
	int pass_len = encoded_passphrase.size();
	char pass[pass_len];
	memset(pass, 0, pass_len);

	for(int i=0; i<pass_len; i++)
		pass[i] = encoded_passphrase[i] ^ ( (i+1) % 16 );

	// use
	JSON_Array params;
	params.push_back( std::string(pass, pass_len) );
	params.push_back( 1 );

	// clean
	memset(pass, 0, pass_len);

	JSON response = server_RPC( "walletpassphrase", &params );
	return response.null("error");
}


std::vector<CryptoCoinTx> BitCoin::get_recent_transactions( std::string account, time_t unix_time, bool include_unconfirmed ) {
	// use "listtransactions" in blocks of 10 until timestamps are less than unix_time
	int batch_start = 0;
	const int batch_size = 10;
	bool more_batches = true;

	std::vector<CryptoCoinTx> transactions;

	while( more_batches ) {
		JSON_Array params;
		params.push_back( "*" );		// NB: deprecated and must be set to "*" until removed from Bitcoin API
		params.push_back( batch_size );
		params.push_back( batch_start );
		params.push_back( true );		// include watch-only addresses

		JSON txs_json = server_RPC( "listtransactions", &params );
		JSON_Array results = txs_json.get_array("result");

		int tx_count = results.size();

		for(int i=0; i<tx_count; i++) {
			JSON tx_json = results.get_item(i);

			// we're only interested in "receive" category transactions (not sends)
			if ( tx_json.get_string("category") != "receive" )
				continue;

			// if transaction's timestamp is earlier than our threshold
			// then this is the last batch we need to look out
			time_t tx_time = tx_json.get_number("time");
			if (tx_time < unix_time) {
				more_batches = false;
				continue;	// continue, not break, because there may be still some qualifying transactions in this batch
			}

			// filter out unconfirmed transaction?
			if ( !include_unconfirmed && tx_json.get_number("confirmations") == 0 )
				continue;

			// transaction is good to go
			CryptoCoinTx tx;
			tx.tx_id = tx_json.get_string("txid");
			#ifdef DEBUG_BITCOIN
				std::cout << "Recent TX: " << tx.tx_id << std::endl;
			#endif

			try {
				tx = get_transaction( tx );
				transactions.push_back( tx );
			} catch (const CryptoCoins::unknown_transaction &e) {
				// ignore transactions that don't exist (for whatever reason)
				continue;
			}
		}

		// if we didn't receive a full batch of transactions then we've had them all
		if (tx_count < batch_size)
			more_batches = false;

		batch_start += batch_size;
	}

	return transactions;
}


CryptoCoinTx BitCoin::get_transaction( CryptoCoinTx info_tx ) {
	JSON transaction_json = get_raw_transaction( info_tx.tx_id );
	if ( transaction_json.null("result") )
		throw CryptoCoins::unknown_transaction();

	JSON result = transaction_json.get_item("result");

	std::string tx_hex = result.get_string("hex");

	// use first vin address as 'sender'
	// fetch transaction ("vin_tx") using vin[0]'s txid
	// use vin[0]'s vout to extract address from vin_tx's vouts
	JSON_Array vin_array = result.get_array("vin");
	JSON vin = vin_array.get_item(0);

	// if there's a "coinbase" then abort early
	if ( vin.exists("coinbase") )
		throw std::runtime_error("Coinbase transaction");

	std::string vin_txid = vin.get_string("txid");
	int vout_index = vin.get_number("vout");

	JSON vin_tx_json = get_raw_transaction( vin_txid );
	JSON vin_tx_result = vin_tx_json.get_item("result");

	JSON_Array vin_tx_vout_array = vin_tx_result.get_array("vout");
	JSON vin_tx_vout = vin_tx_vout_array.get_item(vout_index);

	std::string vin_address = get_vout_address( vin_tx_vout );
		
	if ( vin_address.empty() )
		throw std::runtime_error("No address for vin[0]?");


	// recipients
	JSON_Array vout_array = result.get_array("vout");
	uint64_t vout_size = vout_array.size();
	
	std::vector<CryptoCoinRecvPair> recipient_amounts;
		
	for(int vout_idx = 0; vout_idx < vout_size; vout_idx++) {
		JSON vout = vout_array.get_item(vout_idx);
		
		double amount = vout.get_double("value");
		std::string recipient = get_vout_address( vout );
		
		recipient_amounts.push_back( CryptoCoinRecvPair(recipient, std::lround(amount * amount_divisor)) );
	}


	CryptoCoinTx tx;
	tx.currency = "BTC";
	tx.tx_id = info_tx.tx_id;
	tx.sender = vin_address;

	if ( result.exists("confirmations") )
		tx.confirmations = result.get_number("confirmations");
	else
		tx.confirmations = 0;
		
	if ( result.exists("blockhash") )
		tx.block_height = get_block_height( result.get_string("blockhash") );
	else
		tx.block_height = 0;

	tx.fee = 0; // not supported for now
	tx.fee_inclusive = false;
	tx.recipient_amounts = recipient_amounts;
	
	#ifdef DEBUG_BITCOIN
		std::cout << "TX from " << tx.sender << " to ";

		for(auto const &recv_pair : recipient_amounts)
			std::cout << recv_pair.recipient << " ";

		std::cout << std::endl;
	#endif
	
	if ( result.exists("time") )
		tx.crypto_timestamp = result.get_number("time");
	else
		tx.crypto_timestamp = time(NULL);
	
	tx.unix_timestamp = tx.crypto_timestamp;

	// also SHA256 hash of the transaction contents to help detect replacement before fully confirmed
	SHA256_Data( tx_hex.c_str(), tx_hex.size(), tx.contents_SHA256 );

	return tx;
}


bool BitCoin::send_transaction( CryptoCoinTx &tx ) {
	if ( tx.recipient_amounts.size() != 1 ) {
		std::cout << "Send failed: more than 1 recipient passed" << std::endl;
		return false;
	}

	// unlock wallet
	if ( !unlock_wallet(tx.encoded_passphrase) )
		return false;

	JSON_Array params;
	params.push_back( tx.recipient_amounts[0].recipient );
	params.push_back( (double)(tx.recipient_amounts[0].amount) / (double)(amount_divisor) );
	params.push_back( "" );	// comment
	params.push_back( "" );	// comment-to
	params.push_back( true );	// fee-inclusive

	JSON response = server_RPC("sendtoaddress", &params);
	
	// "result" can be a simple string or a nested JSON
	try {
		std::string tx_id = response.get_string("result");
		
		tx.tx_id = tx_id;
		return true;
	} catch(...) {
		// something went wrong as we didn't get a string-based TX id in "result"
		if ( !response.null("error") ) {
			JSON error = response.get_item("error");

			if ( !error.null("message") ) {
				std::cerr << ftime() << "Bitcoin send failed: " << error.get_string("message") << std::endl;
				return false;
			}
		}
		
		std::cerr << ftime() << "Bitcoin send failed - unknown reason" << std::endl;
		return false;
	}
}


CryptoCoinKeyPair BitCoin::generate_keypair() {
	#ifdef NO_ARC4RANDOM
		throw std::runtime_error("Couldn't generate Bitcoin keypair: no arc4random");
	#else
		uint8_t buffer[1 + 32];
		buffer[0] = 0x80;
		arc4random_buf( &buffer[1], 32 );
		// restrict to valid key range
		buffer[16] &= 0xfc;
		buffer[32] |= 0x01;

		CryptoCoinKeyPair keypair;
		keypair.priv_key = base58check_encode( buffer, 32+1 );

		// generate public key
		BN_CTX *ctx = BN_CTX_new();
	
		const EC_KEY *eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
		const EC_GROUP *group = EC_KEY_get0_group(eckey);
	
		// field is "p"
		BIGNUM *field = BN_CTX_get(ctx);
		EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx);

		// *** actually priv key not r!
		BIGNUM *priv = BN_CTX_get(ctx);
		BN_bin2bn( (const unsigned char *)&buffer[1], 32, priv );

		EC_POINT *xy_point = EC_POINT_new(group);
		EC_POINT_mul(group, xy_point, priv, NULL, NULL, ctx);

		BIGNUM *x_bn = BN_CTX_get(ctx);
		BIGNUM *y_bn = BN_CTX_get(ctx);
		EC_POINT_get_affine_coordinates_GFp(group, xy_point, x_bn, y_bn, ctx);

		// checks
		if (BN_num_bytes(x_bn) != 32 || BN_num_bytes(y_bn) != 32)
			throw std::runtime_error("Couldn't generate Bitcoin keypair?");

		uint8_t pub_buffer[1 + 32 + 32];
		pub_buffer[0] = 0x04;

		BN_bn2bin( x_bn, &pub_buffer[1] );
		BN_bn2bin( y_bn, &pub_buffer[1 + 32] );
	
		
		// SHA256
		uint8_t sha_buf[32];
		SHA256_CTX context;
		SHA256_Init(&context);
		SHA256_Update(&context, pub_buffer, 1 + 32 + 32);
		SHA256_Final(sha_buf, &context);
	

		// RIPEMD160
		uint8_t md160_buf[1 + 20];
		RIPEMD160_CTX md160_ctx;
		RIPEMD160_Init(&md160_ctx);
		RIPEMD160_Update(&md160_ctx, sha_buf, 32);
		RIPEMD160_Final(&md160_buf[1], &md160_ctx);

		md160_buf[0] = 0x00;

		keypair.pub_address = base58check_encode( md160_buf, 21 );

		return keypair;
	#endif
}


std::string BitCoin::base58check_encode(const uint8_t* const str, const uint64_t len) {
	uint8_t buffer[ len + 4 ];
	memcpy(buffer, str, len);

	SHA256_CTX context;

	// double SHA256
	uint8_t sha_buf1[32];
	SHA256_Init(&context);
	SHA256_Update(&context, buffer, len);
	SHA256_Final(sha_buf1, &context);

	uint8_t sha_buf2[32];
	SHA256_Init(&context);
	SHA256_Update(&context, sha_buf1, 32);
	SHA256_Final(sha_buf2, &context);

	// first 4 bytes of double SHA256 output go to &buffer[len]..
	memcpy( &buffer[len], sha_buf2, 4 );

	char *base58;
	base58_encode( &base58, buffer, len+4 );

	std::string out = std::string(base58);
	free(base58);

	return out;
}


std::string BitCoin::pretty_amount( uint64_t amount ) {
	return _pretty_amount(amount, amount_divisor, currency_suffix);
}

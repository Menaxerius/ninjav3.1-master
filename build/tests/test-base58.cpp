#include "Base58.cpp"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <iostream>
#include <string>


int main(int argc, char *argv[]) {
	const int arglen = strlen( argv[1] );
	uint8_t in[arglen >> 1];

	for(int i=0; i<arglen; i+=2) {
		std::string octet( argv[1], i, 2 );
		in[i>>1] = std::stoi( octet, nullptr, 16 );
	}
	

	char *out;
	base58_encode( &out, (const uint8_t *)in, arglen>>1 );

	std::cout << out << std::endl;
}


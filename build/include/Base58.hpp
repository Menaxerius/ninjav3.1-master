#ifndef INCLUDE__BASE58_HPP
#define INCLUDE__BASE58_HPP

#include <stdint.h>

extern "C" {
	uint64_t base58_encode(char** const dest, const uint8_t* const str, const uint64_t len);
}

#endif

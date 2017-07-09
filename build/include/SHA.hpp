#ifndef INCLUDE__SHA_HPP
#define INCLUDE__SHA_HPP

// This is here to provide a shim for SHA256 operations
// as FreeBSD's base sha256.h isn't usable with C++

#include <openssl/sha.h>

char *SHA256_End(SHA256_CTX *context, char *buffer);
char *SHA256_Data(const void *data, unsigned int len, char *buf);

#endif

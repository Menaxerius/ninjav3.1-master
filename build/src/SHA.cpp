#include "SHA.hpp"
#include <string.h>

static const char *sha2_hex_digits = "0123456789abcdef";


char *SHA256_End(SHA256_CTX *context, char *buffer) {
	unsigned char digest[32];

	if (buffer != (char *)NULL) {
		SHA256_Final(digest, context);

		unsigned char *d = digest;
		for (int i=0; i<SHA256_DIGEST_LENGTH; ++i) {
			*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
			*buffer++ = sha2_hex_digits[*d & 0x0f];
			++d;
		}
		*buffer = (char)0;
	} else {
		memset(context, 0, sizeof(*context));
	}

	memset(digest, 0, SHA256_DIGEST_LENGTH);

	return buffer;
}


char *SHA256_Data(const void *data, unsigned int len, char *buf) {
	SHA256_CTX context;
	SHA256_Init(&context);
	SHA256_Update(&context, data, len);
	return SHA256_End(&context, buf);
}

#include "Base58.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

#if 0
	bool b58enc(char *b58, size_t *b58sz, const void *data, size_t binsz)
	{
		const uint8_t *bin = data;
		int carry;
		ssize_t i, j, high, zcount = 0;
		size_t size;
	
		while (zcount < (ssize_t)binsz && !bin[zcount])
			++zcount;
	
		size = (binsz - zcount) * 138 / 100 + 1;
		uint8_t buf[size];
		memset(buf, 0, size);
	
		for (i = zcount, high = size - 1; i < (ssize_t)binsz; ++i, high = j)
		{
			for (carry = bin[i], j = size - 1; (j > high) || carry; --j)
			{
				carry += 256 * buf[j];
				buf[j] = carry % 58;
				carry /= 58;
			}
		}
	
		for (j = 0; j < (ssize_t)size && !buf[j]; ++j);
	
		if (*b58sz <= zcount + size - j)
		{
			*b58sz = zcount + size - j + 1;
			return false;
		}
	
		if (zcount)
			memset(b58, '1', zcount);
		for (i = zcount; j < (ssize_t)size; ++i, ++j)
			b58[i] = b58digits_ordered[buf[j]];
		b58[i] = '\0';
		*b58sz = i + 1;
	
		return true;
	}
#endif

uint64_t base58_encode(char** const dest, const uint8_t* const str, const uint64_t len) {
	size_t n_leading_zeros = 0;
	while( n_leading_zeros < len && str[n_leading_zeros] == 0 )
		++n_leading_zeros;
		
	size_t dest_len = len * 256 / 58 + 1;
	uint8_t buf[dest_len];
	memset(buf, 0, dest_len);

	for (int str_idx = 0; str_idx < len; ++str_idx) {
		uint32_t carry = (uint8_t)str[str_idx];
		
		for (int enc_idx = dest_len - 1; enc_idx >= 0; --enc_idx ) {
			carry += 256 * buf[enc_idx];
			buf[enc_idx] = carry % 58;
			carry /= 58;
		}
	}

	size_t n_leading_ones = 0;
	char counting_ones = 1;

	for (int enc_idx = 0; enc_idx < dest_len; ++enc_idx) {
		if ( counting_ones && buf[enc_idx] == 00 )
			++n_leading_ones;
		else
			counting_ones = 0;

		buf[enc_idx] = b58digits_ordered[ buf[enc_idx] ];
	}

	size_t offset = n_leading_ones - n_leading_zeros;
	dest_len -= offset;

	*dest = (char *)malloc( dest_len );
	if ( *dest == NULL ) {
		perror("Couldn't malloc() space for base58 encoding");
		return 0;
	}

	memcpy(*dest, &buf[offset], dest_len);

	return dest_len;
}

#include "WebSocketFrame.hpp"

#include <iostream>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


void WebSocketFrame::pack(const std::string &payload) {
	opcode = WS_TEXT;
	pack( payload.c_str(), payload.size() );
}


void WebSocketFrame::pack(const char *buffer, const uint64_t buffer_len) {
	if (payload_data != nullptr)
		free(payload_data);

	// determine length of payload buffer needed
	payload_len = buffer_len + 1 + 1;	// flags + 1st octet of frame length

	uint64_t payload_offset = 0;

	if (mask)
		payload_offset += 4;	// 32bit masking key

	if (buffer_len > 65536)
		payload_offset += 8;	// int64_t payload length
	else if (buffer_len > 125)
		payload_offset += 2;	// uint16_t payload length

	payload_len += payload_offset;

	// now we can allocate space
	payload_data = (char *)malloc(payload_len);
	if (payload_data == nullptr)
		throw std::runtime_error("Can't malloc() space for websocket frame");

	memset(payload_data, 0, payload_len);

	// flags
	payload_data[0] = (fin << 7) | (rsv1 << 6) | (rsv2 << 5) | (rsv3 << 4) | (opcode & 0xF);

	if (buffer_len > 65536) {
		payload_data[1] = 127;
		*(uint32_t *)( &payload_data[2] ) = htonl(buffer_len >> 32);
		*(uint32_t *)( &payload_data[6] ) = htonl(buffer_len & 0xFFFFFFFF);
	} else if (buffer_len > 125) {
		payload_data[1] = 126;
		*(uint16_t *)(&payload_data[2]) = htons(buffer_len);
	} else {
		payload_data[1] = buffer_len;
	}

	if (!mask) {
		// simple copy
		memcpy( &payload_data[2 + payload_offset], buffer, buffer_len);
	} else {
		payload_data[1] |= (mask << 7);

		#ifdef NO_ARC4RANDOM
			masking_key = random();
		#else
			masking_key = arc4random();
		#endif

		*(uint32_t *)( &payload_data[2 + payload_offset - 4] ) = htonl(masking_key);

		uint8_t *masking_key_p = (uint8_t *)&masking_key;

		for(uint64_t i=0; i<buffer_len; i++)
			payload_data[2 + payload_offset + i] = buffer[i] ^ masking_key_p[3 - (i % 4)];
	}

	// debugging
	#ifdef WSF_PACK_DEBUG
		for(uint64_t i=0; i<payload_len; i+=16) {
			fprintf(stderr, "%08lx  ", i);
			for(int j=0; j<16 && i+j<payload_len; j++) {
				fprintf(stderr, "%02x ", (uint8_t)payload_data[i+j]);
			}
			fprintf(stderr, "  ");
			for(int j=0; j<16 && i+j<payload_len; j++) {
				uint8_t c = payload_data[i+j];
				fprintf(stderr, "%c", (c>=32 && c<=126 ? c : '.'));
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		}
	#endif
}


WebSocketFrame::~WebSocketFrame() {
	if (payload_data != nullptr)
		free(payload_data);
}


uint64_t WebSocketFrame::unpack(const char *buffer, const uint64_t buffer_len) {
	if (buffer_len == 0)
		return 0;

	// debugging
	#ifdef WSF_UNPACK_DEBUG
		for(int i=0; i<buffer_len; i+=16) {
			fprintf(stderr, "%08x  ", i);
			for(int j=0; j<16 && i+j<buffer_len; j++) {
				fprintf(stderr, "%02x ", (uint8_t)buffer[i+j]);
			}
			fprintf(stderr, "  ");
			for(int j=0; j<16 && i+j<buffer_len; j++) {
				uint8_t c = buffer[i+j];
				fprintf(stderr, "%c", (c>=32 && c<=126 ? c : '.'));
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		}
	#endif

	fin = !!(buffer[0] & 0x80);
	rsv1 = !!(buffer[0] & 0x40);
	rsv2 = !!(buffer[0] & 0x20);
	rsv3 = !!(buffer[0] & 0x10);

	opcode = static_cast<opcode_enum>(buffer[0] & 0x0f);

	// we don't handle extensions or reserved opcodes
	if (rsv1 || rsv2 || rsv3 || ( opcode >= WS_RSV3 && opcode <= WS_RSV7 ) || ( opcode >= WS_CRSVB && opcode <= WS_CRSVF ) )
		return 0;

	if (buffer_len < 2)
		return 0;

	mask = !!(buffer[1] & 0x80);
	payload_len = buffer[1] & 0x7f;

	uint64_t offset = 2;

	if (payload_len == 126) {
		if (buffer_len < offset+2)
			return 0;

		payload_len = ntohs( *(uint16_t *)(&buffer[offset]) );
		offset += 2;
	} else if (payload_len == 127) {
		if (buffer_len < offset+8)
			return 0;

		// websocket framing is MSB so first 32bits are upper
		payload_len = ntohl( *(uint32_t *)(&buffer[offset]) );
		payload_len <<= 32;
		offset += 4;
		payload_len += ntohl( *(uint32_t *)(&buffer[offset]) );
		offset += 4;
	}

	if (mask) {
		if (buffer_len < offset+4)
			return 0;

		masking_key = ntohl( *(uint32_t *)(&buffer[offset]) );
		offset += 4;
	}

	if (buffer_len < offset+payload_len)
		return 0;

	payload_data = (char *)malloc(payload_len);
	memcpy(payload_data, &buffer[offset], payload_len);

	if (mask) {
		uint8_t *masking_key_p = (uint8_t *)&masking_key;

		for(uint64_t i=0; i<payload_len; i++) {
			payload_data[i] = payload_data[i] ^ masking_key_p[3 - (i % 4)];
		}
	}

	if (opcode == WS_CLOSE && payload_len == 2) {
		close_code = ntohs( *(uint16_t *)payload_data );
	}

	offset += payload_len;

	return offset;
}

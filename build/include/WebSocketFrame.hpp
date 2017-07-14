#ifndef INCLUDE__WEBSOCKETFRAME_HPP
#define INCLUDE__WEBSOCKETFRAME_HPP

#include <bsd/stdlib.h>
#include <stdint.h>
#include <string>

class WebSocketFrame {
	public:
		enum opcode_enum : uint8_t {
			WS_CONT		= 0x0,
			WS_TEXT		= 0x1,
			WS_BIN		= 0x2,
			WS_RSV3		= 0x3,
			WS_RSV4		= 0x4,
			WS_RSV5		= 0x5,
			WS_RSV6		= 0x6,
			WS_RSV7		= 0x7,
			WS_CLOSE	= 0x8,
			WS_PING		= 0x9,
			WS_PONG		= 0xA,
			WS_CRSVB	= 0xB,
			WS_CRSVC	= 0xC,
			WS_CRSVD	= 0xD,
			WS_CRSVE	= 0xE,
			WS_CRSVF	= 0xF
		};

		bool			fin = true;
		bool			rsv1 = false;
		bool			rsv2 = false;
		bool			rsv3 = false;
		bool			mask = false;
		uint32_t		masking_key = 0;
		uint64_t		payload_len = 0;
		char			*payload_data = nullptr;
		uint16_t		close_code = 0;
		opcode_enum		opcode = WS_CONT;

		~WebSocketFrame();

		void pack(const std::string &payload);
		void pack(const char *buffer, const uint64_t buffer_len);
		uint64_t unpack(const char *buffer, const uint64_t buffer_len);
};

#endif

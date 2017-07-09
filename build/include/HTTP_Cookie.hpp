#ifndef INCLUDE__HTTP_COOKIE_HPP
#define INCLUDE__HTTP_COOKIE_HPP

#include <string>
#include <time.h>

class HTTP_Cookie {
	public:
		std::string					name;
		std::string					value;
		time_t						expires;
		std::string					path;
		std::string					domain;
		bool						secure;
		bool						http_only;
		
		HTTP_Cookie();

		std::string to_string();
};

#endif

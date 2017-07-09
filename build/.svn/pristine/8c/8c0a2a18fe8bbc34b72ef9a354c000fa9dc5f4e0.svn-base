#ifndef INCLUDE__EMAIL_HPP
#define INCLUDE__EMAIL_HPP

#include <string>
#include <map>


class Email {
	private:
		void write_string( int fd, const std::string &data );

	protected:
		int fds[2] = { 0, 0 };
		int pid;
		bool sent_headers;

	public:
		std::map<std::string, std::string> headers;
	
		bool send_headers();
		bool send_body( const std::string &content );
		void give_up();

		static bool validate( const std::string &email );
};

#endif

#include "Email.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <locale>
#include <regex>

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>


void Email::write_string( int fd, const std::string &data ) {
	int n_bytes = write(fd, data.c_str(), data.length() );

	if ( n_bytes != data.length() )
		throw std::runtime_error("Couldn't write all bytes to sendmail");
}


bool Email::send_headers( ) {
	// extract from_name and from_email from From: header!
	std::string from_name;
	std::string from_email;
	
	std::smatch matches;

	std::regex_match( headers["From"], matches, std::regex( "^([^<]+)<([^>]+)>$" ) );

	if ( matches.size() == 3 ) {
		// name <email> form
		from_name = matches[1].str();
		from_email = matches[2].str();
	} else {
		from_email = headers["From"];
	}

	// connect to sendmail...
	int ret = pipe( fds );
	if ( ret == -1 ) {
		// log and exit
		perror("Couldn't create pipe for sendmail");
		return false;
	}
	
	pid = vfork();
	if ( pid == -1 ) {
		perror("Couldn't fork for sendmail");
		close( fds[0] );
		close( fds[1] );
		return false;
	}
	
	if ( !pid ) {
		// child
		
		// close write end of pipe - we're reading only
		close(fds[1]);
		
		// remap read end of pipe to stdin
		dup2(fds[0], STDIN_FILENO);
		
		// set up args
		const std::vector<std::string> argv_s { "sendmail", "-L", "web-sent-email", "-O", "IgnoreDots", "-O", "DeliveryMode=q", "-F", from_name, "-f", from_email, headers["To"] };

		std::vector<char *> argv;
		
		for(int j=0; j<argv_s.size(); j++)
			argv.push_back( const_cast<char *>( argv_s[j].c_str() ) );
		argv.push_back( NULL );

		char *envp[] = { NULL };
		
		execve("/usr/local/sbin/sendmail", &argv[0], envp);
		
		perror("Couldn't exec sendmail");
		
		_exit(2);
	}

	// vfork() blocks parent with child 
	// so we get here only if execve() has loaded sendmail or _exit() has been called?
	
	int status;
	int ret_pid = waitpid( pid, &status, WNOHANG | WEXITED );
	
	if ( ret_pid == -1 || ( WIFEXITED(status) && WEXITSTATUS(status) != 0 ) ) {
		// something bad happened
		close(fds[0]);
		close(fds[1]);
		return false;
	}


	// close read end of pipe - we're writing only
	close( fds[0] );
	
	try {
		for(auto it : headers) {
			write_string( fds[1], it.first );
			write_string( fds[1], ": " );
			write_string( fds[1], it.second );
			write_string( fds[1], "\r\n" );
			fsync( fds[1] );
		}
	} catch(...) {
		return false;
	}
	
	return true;
}


bool Email::send_body( const std::string &content ) {
	try {
		write_string( fds[1], content );
		write_string( fds[1], "\r\n" );
		fsync( fds[1] );
	} catch(...) {
		return false;
	}

	close( fds[1] );

	// wait for sendmail as it's only queuing so should be quick
	#ifndef WEXITED
	// FreeBSD 9.x compat
	#define WEXITED 0
	#endif

	int status;
	int ret_pid = waitpid( pid, &status, WEXITED );
	
	if ( ret_pid == -1 ) {
		perror("Couldn't call waitpid for sendmail child");
	} else {
		if ( WIFEXITED(status) && WEXITSTATUS(status) == 0 ) {
			// all good!
			return true;
		} 
	}
	
	// oh well
	return false;
}


void Email::give_up() {
	// kill sendmail :(
	int ret = kill( pid, SIGTERM );
	if ( ret == -1 )
		perror("Couldn't kill sendmail child");
	
	
	// wait for exit 
	int status;
	int ret_pid = waitpid( pid, &status, WEXITED );
	
	if ( ret_pid == -1 )
		perror("Couldn't call waitpid for sendmail child");

	// close pipe
	close( fds[1] );
}


bool Email::validate( const std::string &email ) {
	std::smatch matches;

	std::regex_match( email, matches, std::regex( "^([^:;@<>]+)@([a-zA-Z0-9._-]+)$" ) );
	if ( matches.size() != 3 )
		return false;

	// check MX
	unsigned char answer[1500];
	int ret = res_query( matches[2].str().c_str(), C_IN, T_MX, answer, sizeof(answer) );

	if (ret == -1)
		return false;

	HEADER *hdr = reinterpret_cast<HEADER*>(answer);

	if (hdr->rcode != NOERROR)
		return false;

	// no answers? no go!
	if (hdr->ancount == 0)
		return false;

	return true;
}

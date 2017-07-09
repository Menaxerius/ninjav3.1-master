#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <regex>

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <unistd.h>


std::string read_file(const std::string &src_filename) {
	try {
		std::ifstream fs;
		fs.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
		fs.open(src_filename, std::ifstream::in | std::ifstream::ate);

		auto size = fs.tellg();

		auto buffer = std::make_unique<char[]>( size );

		fs.seekg(0);
		fs.read( buffer.get(), size );
		fs.close();

		return std::string( buffer.get(), size );
	} catch(const std::ifstream::failure& e) {
		std::cerr << "Can't open " << src_filename << ": " << strerror(errno) << std::endl;
		exit(2);
	}
}


int main(int argc, char *argv[]) {
	std::vector<std::string> args( argv + 1, argv + argc );

	if ( argc <= 1 ) {
		std::cerr << "usage: " << argv[0] << " object.hpp [object.hpp ...]" << std::endl;
		exit(1);
	}

	for( std::string src_filename : args ) {
		std::string str = read_file(src_filename);

		// locate "create table" comment
		std::regex create_table_RE( "/\\*"  "\\s+"  "CREATE TABLE "  "(\\w+)"  " \\("  "((?:.|[\n\r])*)"  "\\);"  "(?:.|[\n\r])*?"  "\\*/", std::regex::icase );
		std::smatch smatches;

		std::regex_search(str, smatches, create_table_RE );

		if (smatches.size() != 3) {
			std::cerr << "Couldn't find SQL 'CREATE TABLE' statement" << std::endl;
			exit(2);
		}

		std::string table_name( smatches[1] );
		std::string create_table( smatches[2] );

		std::cout << "DROP TABLE IF EXISTS " << table_name << ";" << std::endl;
		std::cout << "CREATE TABLE " << table_name << " (" << create_table << ");" << std::endl;
	}
}

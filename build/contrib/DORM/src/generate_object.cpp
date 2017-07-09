#include "../templates/object_info.hpp"

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

#ifndef TMP_DIR
#define TMP_DIR "/tmp"
#endif


// order is important
const std::list< std::pair<std::string, std::string> > SQL_CXXTYPE_PAIRS = {
		{ "decimal",			"double" },
		{ "double",				"double" },
		{ "float",				"double" },
		{ "tinyint\\(1\\)",		"bool" },
		{ "bigint unsigned",	"uint64_t" },
		{ "bigint",				"int64_t" },
		{ "int unsigned",		"uint32_t" },
		{ "int",				"int32_t" },
		{ "varchar",			"std::string" },
		{ "char",				"std::string" },
		{ "text",				"std::string" },
		// varbinary, binary, blob unsupported for now
		{ "enum",				"std::string" },
		{ "serial",				"uint64_t" },
		{ "timestamp\\(",		"DORM::Timestamp" },
		{ "timestamp",			"DORM::Timestamp" },
		{ "boolean",			"bool" }
};


std::map<std::string, std::string> CONN_BY_CXX = {
		{ "double",				"Double" },
		{ "uint64_t", 			"UInt64" },
		{ "int64_t",			"Int64" },
		{ "uint32_t",			"UInt" },
		{ "int32_t",			"Int" },
		{ "std::string",		"String" },
		{ "DORM::Timestamp",	"String" },
		{ "bool",				"Boolean" }
};


std::string b2s(const bool &b) {
	return b ? "true" : "false";
}


std::string join( const std::vector<std::string> &strings, const std::string &separator, const std::string &enclose = "" ) {
	bool first = true;
	std::string output;

	for(const auto &str : strings) {
		if (!first)
			output += separator;

		output += enclose + str + enclose;
		first = false;
	}

	return output;
}


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


void log_navigator(const Info::Navigator &navigator) {
	std::cout << "Navigator to " << navigator.object << (navigator.plural ? "(s)" : "") << " (#include " << navigator.filename << ") via " << navigator.function << "()" << std::endl;
}


std::vector<std::string> split(std::string str) {
	std::vector<std::string> parts;
	std::smatch smatches;

	while( std::regex_search( str, smatches, std::regex("(\\w+)"  "(?:,?\\s*)") ) ) {
		parts.push_back( smatches[1] );
		str = smatches.suffix().str();
	}

	return parts;
}


std::string snake_to_camel(std::string snake) {
	std::string camel;
	std::smatch matches;

	while( std::regex_search( snake, matches, std::regex("(_.|.)") ) ) {
		std::string match( matches[1] );
		snake = matches.suffix().str();

		if ( match.size() == 1 )
			camel += match;
		else
			camel += toupper( match[1] );
	}

	// force first char to uppercase
	camel[0] = toupper( camel[0] );

	return camel;
}


std::string snake_to_filename(std::string snake) {
	std::string filename;
	std::smatch matches;

	while( std::regex_search( snake, matches, std::regex("(_.|.)") ) ) {
		std::string match( matches[1] );
		snake = matches.suffix().str();

		if ( match.size() == 1 )
			filename += match;
		else {
			filename += "/";
			filename += static_cast<char>( toupper( match[1] ) );
		}
	}

	// force first char to uppercase
	filename[0] = toupper( filename[0] );

	return filename + ".hpp";
}


void parse(const std::string &str, Info &info) {
	// locate "create table" comment
	std::regex create_table_RE( "/\\*"  "\\s+"  "CREATE TABLE "  "(\\w+)"  " \\("  "((?:.|[\n\r])*)"  "\\);"  "(?:.|[\n\r])*?"  "\\*/", std::regex::icase );
	std::smatch smatches;

	std::regex_search(str, smatches, create_table_RE );

	if (smatches.size() != 3) {
		std::cerr << "Couldn't find SQL 'CREATE TABLE' statement" << std::endl;
		exit(2);
	}

	const std::string post_create_table = smatches.suffix().str();


	info.table_name = smatches[1];
	std::cout << "Table name: " << info.table_name << std::endl;


	std::string create_table( smatches[2] );

	// now parse create_table content
	std::regex create_line_RE( "\\s*"  "(.*?)"  ",?"  "\\s*"  "(?:#.*)?"  "\\n" );
	while( std::regex_search(create_table, smatches, create_line_RE) ) {
		const std::string create_line = smatches[1];
		std::cout << "Stanza: " << create_line << std::endl;

		create_table = smatches.suffix().str();

		std::smatch line_smatches;

		// check for non-column-definitions first

		// PRIMARY KEY
		std::regex primary_key_RE( "PRIMARY KEY"  "\\s+"  "\\("  "([^\\)]+)"  "\\)", std::regex::icase );

		if ( std::regex_search(create_line, line_smatches, primary_key_RE) ) {
			info.keys = split( line_smatches[1] );

			std::cout << "Found PRIMARY KEY stanza, columns (" << info.keys.size() << "): " << join(info.keys, ", ") << std::endl;

			for(const auto &key_column : info.keys)
				for(auto &column : info.columns)
					if (column.name == key_column) {
						column.is_key = true;
						break;
					}

			continue;
		}

		// INDEX
		std::regex index_RE( "INDEX"  "\\s+"  "\\("  "([^\\)]+)"  "\\)", std::regex::icase );

		if ( std::regex_search(create_line, line_smatches, index_RE) ) {
			std::vector<std::string> index = split( line_smatches[1] );
			info.indexes.push_back(index);

			std::cout << "Found INDEX stanza, columns (" <<  index.size() << "): " << join(index, ", ") << std::endl;

			continue;
		}

		// assuming standard column definition from here on
		std::regex column_RE( "(\\w+)"  "\\s+"  "(.*)", std::regex::icase );

		if ( !std::regex_search(create_line, line_smatches, column_RE) ) {
			std::cerr << "Found non-standard column definition within CREATE TABLE" << std::endl;
			exit(2);
		}

		Info::Column column;
		column.name = line_smatches[1];

		std::string column_def = line_smatches[2];

		column.not_null = std::regex_search(column_def, std::regex("NOT NULL", std::regex::icase) );
		column.has_default = std::regex_search(column_def, std::regex("DEFAULT", std::regex::icase) );

		#if 0
			// TIMESTAMP columns have implicit DEFAULT?
			if ( std::regex_search(column_def, std::regex("timestamp\\(", std::regex::icase) ) )
				column.has_default = true;
		#endif

		// try to map column type
		for( const auto &pair : SQL_CXXTYPE_PAIRS ) {
			if ( std::regex_search(column_def, std::regex(pair.first, std::regex::icase) ) ) {
				column.cxxtype = pair.second;
				column.conntype = CONN_BY_CXX[column.cxxtype];

				std::cout << "Column: " << column.name << ", matches SQL: " << pair.first << ", so CXXTYPE: " << pair.second << ", CONNTYPE: " << column.conntype;

				if (column.not_null)
					std::cout << ", NOT_NULL";

				if (column.has_default)
					std::cout << ", HAS_DEFAULT";

				// also check for AUTO_INCREMENT or SERIAL so we can set autoinc_index
				if ( pair.first == "serial" || std::regex_search( column_def, std::regex("AUTO_INCREMENT", std::regex::icase) ) ) {
					if (info.autoinc_index != 0) {
						std::cerr << "Auto-increment index already set to " << info.autoinc_index << std::endl;
						exit(2);
					}

					info.autoinc_index = info.columns.size() + 1;	// NB: BEFORE pushing our latest column!

					std::cout << ", AUTO_INCREMENT";
				}


				std::cout << std::endl;
				break;
			}
		}

		if ( column.cxxtype.empty() ) {
			std::cerr << "Couldn't match column definition to CXXTYPE: " << column_def << std::endl;
			exit(2);
		}

		column.index = info.columns.size() + 1;	// NB: BEFORE pushing our latest column!
		info.columns.push_back(column);


		// if column name ends with "ID" then generate navigator
		if ( column.name.size() > 2 && column.name.substr(column.name.size() - 2) == "ID" ) {
			Info::Navigator navigator;
			navigator.function = column.name.substr(0, column.name.size() - 2);
			navigator.plural = false;
			navigator.object = snake_to_camel(navigator.function);	// already stripped of trailing ID
			navigator.filename = snake_to_filename(navigator.function);	// already stripped of trailing ID

			info.navigators.push_back(navigator);

			log_navigator(navigator);
		}
	}


	// search post-create-table for other special pseudo-#DEFINEs like CHILD_OBJECTS


	// CHILD_OBJECT[S]
	std::regex child_objects_RE( "\n\\s*"  "CHILD_OBJECT(S?)"  "\\("  "(\\w+)"  ", "  "(\\w+)"  "\\);?" );

	std::string child_str = post_create_table;
	while( std::regex_search( child_str, smatches, child_objects_RE) ) {
		Info::Navigator navigator;
		navigator.object = smatches[2];
		navigator.function = smatches[3];
		navigator.plural = !std::string(smatches[1]).empty();
		navigator.filename = std::regex_replace( navigator.object, std::regex( "(.)([A-Z])" ), "$1/$2" ) + ".hpp";

		info.navigators.push_back(navigator);

		std::cout << "CHILD ";
		log_navigator(navigator);

		child_str = smatches.suffix().str();
	}
}


void validate(Info &info)  {
	// check navigators
	for(int i = 0; i < info.navigators.size(); ++i) {
		auto &navigator = info.navigators[i];

		if (navigator.object == info.class_name) {
			std::cout << "Removing self-referential navigator " << navigator.function << "() to " << navigator.object << " (us)" << std::endl;
			info.navigators.erase( info.navigators.begin() + i );
			--i;
			continue;
		}
	}
}


void process_template(std::string &template_str) {
	// <%= ... %>
	template_str = std::regex_replace( template_str, std::regex("<%=(.+?)%>"), "<% std::cout << $1; %>" );

	// <% ... %>
	template_str = std::regex_replace( template_str, std::regex("%>((?:.|\n)+?)<%"), "; std::cout << R\"RAW($1)RAW\"; " );

	// start-of-string ... <%
	template_str = std::regex_replace( template_str, std::regex("((?:.|\n)+?)<%"), "\tstd::cout << R\"RAW($1)RAW\"; ", std::regex_constants::format_first_only );

	// %> ... end-of-string
	template_str = std::regex_replace( template_str, std::regex("%>((?:.|\n)+)"), "; std::cout << R\"RAW($1)RAW\"; ", std::regex_constants::format_first_only );
}


void write_template( const std::string &template_filename, const Info &info, const std::string &tmp_filename ) {
	std::string template_str = read_file(template_filename);

	// process template
	process_template(template_str);

	// output
	std::string dst_filename = tmp_filename + ".cpp";
	std::cout << "Writing template " << template_filename << " to " << dst_filename << std::endl;

	try {
		std::ofstream fs;
		fs.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
		fs.open(dst_filename, std::ofstream::out | std::ofstream::trunc);

		// prologue
		std::string prologue = R"PROLOGUE(
			#include "object_info.hpp"
			#include <iostream>

			int main(int argc, char *argv[]) {
		)PROLOGUE";

		fs.write( &prologue[0], prologue.size() );

		// write Info object
		fs << "Info info = {\n";
		fs << "\"" << info.class_name << "\", // class name\n";
		fs << "\"" << info.table_name << "\", // table name\n";
		fs << "\"" << info.basename << "\", // basename\n";
		// keys
		fs << "{ " << join(info.keys, ", ", "\"") << " }, // keys\n";
		// indexes
		fs << "{ // indexes\n";
		for(int i=0; i<info.indexes.size(); ++i)
			fs << "\t{ " << join(info.indexes[i], ", ", "\"") << " }" << (i != info.indexes.size() - 1 ? "," : "") << "\n";
		fs << "},\n";
		// columns
		fs << "{ // columns\n";
		for(int i=0; i<info.columns.size(); ++i) {
			const auto &column = info.columns[i];
			fs << "\t{ \"" << column.name << "\", ";
			fs << column.index << ", ";
			fs << b2s(column.is_key) << ", ";
			fs << b2s(column.not_null) << ", ";
			fs << b2s(column.has_default) << ", ";
			fs << "\"" << column.cxxtype << "\", ";
			fs << "\"" << column.conntype << "\" }";
			if (i != info.columns.size() - 1)
				fs << ",";
			fs << "\n";
		}
		fs << "},\n";
		// navigators
		fs << "{ // navigators\n";
		for(int i=0; i<info.navigators.size(); ++i) {
			const auto &navigator = info.navigators[i];
			fs << "\t{ \"" << navigator.object << "\", ";
			fs << "\"" << navigator.function << "\", ";
			fs << b2s(navigator.plural) << ", ";
			fs << "\"" << navigator.filename << "\" }";
			if (i != info.navigators.size() - 1)
				fs << ",";
			fs << "\n";
		}
		fs << "},\n";
		// autoinc
		fs << info.autoinc_index << ", // auto_increment index\n";
		// key_params
		fs << "\"" << info.key_params << "\" // key parameters\n";
		// done!
		fs << "};\n";

		fs.write( &template_str[0], template_str.size() );

		// epilogue
		std::string epilogue = R"EPILOGUE(
			}
		)EPILOGUE";

		fs.write( &epilogue[0], epilogue.size() );

		fs.close();
	} catch(const std::ofstream::failure& e) {
		std::cerr << "Can't create " << dst_filename << ": " << strerror(errno) << std::endl;
		exit(2);
	}
}


int main(int argc, char *argv[]) {
	std::vector<std::string> args( argv + 1, argv + argc );

	if ( argc <= 1 || (args[0] == "-d" && argc <= 3) ) {
		std::cerr << "usage: " << argv[0] << " [-d output-dir] object.hpp [object.hpp ...]" << std::endl;
		exit(1);
	}

	std::string output_dir;

	if ( args[0] == "-d") {
		output_dir = args[1] + "/";
		args.erase( args.begin(), args.begin() + 2 );
	}

	for( std::string src_filename : args ) {
		std::string str = read_file(src_filename);

		// do the hard work
		Info info;
		info.autoinc_index = 0;		// no known auto_increment column

		// determine actual class name
		std::smatch smatches;

		if ( !std::regex_match(src_filename, smatches, std::regex("(?:|.*?/)"  "([A-Z].+)"  "\\.[ch]pp") ) ) {
			std::cerr << "Can't convert filename '" << src_filename << "' to class name" << std::endl;
			exit(2);
		}

		info.basename = smatches[1];
		std::cout << "Base name: " << info.basename << std::endl;

		std::string class_name = smatches[1];
		info.class_name = std::regex_replace( class_name, std::regex("/"), "" );

		std::cout << "Class name: " << info.class_name << std::endl;

		// generate info
		parse(str, info);

		// validate info
		validate(info);

		// generate handy key_params
		std::string separator;
		for(const auto &column : info.columns)
			if (column.is_key) {
				info.key_params += separator + column.cxxtype + " key_" + column.name;
				separator = ", ";
			}

		// split out files?
		const std::vector<std::string> template_names( { "object_.hxx", "object_.cxx" } );

		std::string template_dir = std::regex_replace( argv[0], std::regex("bin/[^/]+"), "templates/" );

		for( const auto &template_name : template_names ) {
			char tmp_fmt[] = TMP_DIR "/DORM-XXXXXX";
			mktemp(tmp_fmt);

			const std::string tmp_filename( tmp_fmt );

			write_template( template_dir + template_name, info, tmp_filename );

			// compile
			std::cout << "Compiling template " << tmp_filename << ".cpp to create " << tmp_filename << ".aout" << std::endl;
			std::string command = "c++ -std=c++14 -g -pipe -O0 -Wall -I. -I" + template_dir + " -o " + tmp_filename + ".aout " + tmp_filename + ".cpp";
			system( command.c_str() );

			std::string dst_filename = output_dir + info.basename + std::regex_replace( template_name, std::regex("object"), "" );

			// use template
			std::cout << "Using template " << tmp_filename << ".aout to create " << dst_filename << std::endl;
			command = "mkdir -p `dirname " + dst_filename + "`";
			system( command.c_str() );

			command = tmp_filename + ".aout > " + dst_filename;
			system( command.c_str() );

			std::string unlink_filename = tmp_filename + ".cpp";
			unlink( unlink_filename.c_str() );
			unlink_filename = tmp_filename + ".aout";
			unlink( unlink_filename.c_str() );
		}
	}
}

#ifndef INCLUDE__JSON_HPP
#define INCLUDE__JSON_HPP

#include "cJSON.hpp"

#include <stdexcept>
#include <string>


class JSON_Array;

class JSON {
	friend class JSON_Array;

	private:
		cJSON *cjson;
		bool is_root;

		cJSON *_get_item( const std::string name ) const;

		static bool _get_boolean( const cJSON *item );
		static int _get_number( const cJSON *item );
		static double _get_double( const cJSON *item );
		static int64_t _get_int64( const cJSON *item );
		static uint64_t _get_uint64( const cJSON *item );
		static std::string _get_string( const cJSON *item );
		static JSON _get_item( cJSON *item );
		static JSON_Array _get_array( cJSON *item );

	public:
		class parse_error: public std::runtime_error {
			public:
				parse_error(): std::runtime_error("cJSON couldn't parse") {};
		};


		JSON() : is_root(true) {
			cjson = cJSON_CreateObject();
		};
		
		JSON( const std::string json_to_parse ) : is_root(true) {
			cjson = cJSON_Parse( json_to_parse.c_str() );
			if (!cjson)
				throw parse_error();
		};
		
		JSON( const char *json_to_parse ) : is_root(true) {
			cjson = cJSON_Parse( json_to_parse );
			if (!cjson)
				throw parse_error();
		};

		JSON( cJSON *new_cjson, bool new_is_root ) : cjson(new_cjson), is_root(new_is_root) {};

		JSON( JSON &&other_json ): cjson(other_json.cjson), is_root(other_json.is_root) {
			other_json.cjson = nullptr;
			other_json.is_root = false;
		};

		JSON(const JSON &other_json): is_root(true) {
			cjson = cJSON_Duplicate(other_json.cjson, true);
		};

		JSON &operator=(const JSON &other_json) {
			if (this == &other_json)
			return *this;

			if (this->is_root && this->cjson)
				cJSON_Delete(this->cjson);

			this->cjson = cJSON_Duplicate(other_json.cjson, true);
			this->is_root = true;

			return *this;
		};

		JSON &operator=(JSON &&other_json) {
			if (this->cjson && this->is_root)
				cJSON_Delete(this->cjson);

			this->cjson = other_json.cjson;
			this->is_root = other_json.is_root;

			other_json.cjson = nullptr;
			other_json.is_root = false;

			return *this;
		};

		~JSON() {
			// is our root THE root?
			if (is_root && cjson)
				cJSON_Delete(cjson);
		};
		

		bool exists( const std::string name ) const;
		bool null( const std::string name ) const;
		bool empty() const;

		bool get_boolean( const std::string name ) const;
		int get_number( const std::string name ) const;
		double get_double( const std::string name ) const;
		int64_t get_int64( const std::string name ) const;
		uint64_t get_uint64( const std::string name ) const;
		std::string get_string( const std::string name ) const;
		JSON get_item( const std::string name ) const;
		JSON_Array get_array( const std::string name ) const;

		void add_boolean( const std::string name, const bool value );
		void add_number( const std::string name, const int value );
		void add_number( const std::string name, const unsigned int value );
		void add_number( const std::string name, const double value );
		void add_int( const std::string name, const int value );
		void add_uint( const std::string name, const unsigned int value );
		void add_double( const std::string name, const double value );
		void add_int64( const std::string name, const int64_t value );
		void add_uint64( const std::string name, const uint64_t value );
		void add_string( const std::string name, const std::string value );
		void add_item( const std::string name, JSON &json );
		void add_item( const std::string name, JSON_Array &json_array );
		
		void delete_item( const std::string &name );

		std::string to_string();
};


class JSON_Array {
	friend class JSON;

	private:
		cJSON *cjson;
		bool is_root;

		cJSON *_get_item( const int index ) const;

	public:
		JSON_Array() : is_root(true) {
			cjson = cJSON_CreateArray();
		};
		
		JSON_Array( const std::string json_to_parse ) : is_root(true) {
			cjson = cJSON_Parse( json_to_parse.c_str() );
			if (!cjson)
				throw JSON::parse_error();
		};

		JSON_Array( cJSON *new_cjson, bool new_is_root );

		~JSON_Array() {
			// is our root THE root?
			if (is_root && cjson)
				cJSON_Delete(cjson);
		};

		// move constructor
		explicit JSON_Array(JSON &&json) {
			if (is_root && cjson)
				cJSON_Delete(cjson);

			is_root = json.is_root;
			cjson = json.cjson;

			// don't let original destructor wipe out cjson
			json.is_root = false;
			json.cjson = nullptr;
		};


		// copy constructor
		JSON_Array(const JSON_Array &source) : is_root(true) {
			cjson = cJSON_Duplicate( source.cjson, 1 );
		};


		void push_back( bool value );
		void push_back( int number );
		void push_back( unsigned int number );
		void push_back( double number );
		void push_back( int64_t number );
		void push_back( uint64_t number );
		void push_back( const char *str );
		void push_back( std::string str );
		void push_back( JSON &item );
		void push_back( JSON_Array &item_array );

		void shift();

		int size() const;

		bool get_boolean( const int index ) const;
		int get_number( const int index ) const;
		double get_double( const int index ) const;
		int64_t get_int64( const int index ) const;
		uint64_t get_uint64( const int index ) const;
		std::string get_string( const int index ) const;
		JSON get_item( const int index ) const;
		JSON_Array get_array( const int index ) const;

		std::string to_string();

		void clear();
};

#endif

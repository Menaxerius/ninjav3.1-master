#include "JSON.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include <iostream>


// JSON //

cJSON *JSON::_get_item( const std::string name ) const {
	cJSON *item = cJSON_GetObjectItem( cjson, name.c_str() );
	if (!item)
		throw std::runtime_error("cJSON item '" + name + "' not found");

	return item;
}


bool JSON::_get_boolean( const cJSON *item ) {
	if (item->type != cJSON_False && item->type != cJSON_True && item->type != cJSON_Number)
		throw std::runtime_error("cJSON item not a boolean or int");

	return item->type == cJSON_True || item->valueint != 0;
}


int JSON::_get_number( const cJSON *item ) {
	if (item->type == cJSON_String)
		try {
			return std::stoi( item->valuestring );
		} catch(...) {
			throw std::runtime_error("cJSON item not a number");
		}

	if (item->type != cJSON_Number)
		throw std::runtime_error("cJSON item not a number");

	return item->valueint;
}


double JSON::_get_double( const cJSON *item ) {
	if (item->type == cJSON_String)
		try {
			return std::stod( item->valuestring );
		} catch(...) {
			throw std::runtime_error("cJSON item not a double");
		}

	if (item->type != cJSON_Number)
		throw std::runtime_error("cJSON item not a number");

	return item->valuedouble;
}


int64_t JSON::_get_int64( const cJSON *item ) {
	if (item->type == cJSON_String)
		try {
			return std::stoll( item->valuestring );
		} catch(...) {
			throw std::runtime_error("cJSON item not a number");
		}

	if (item->type != cJSON_Number)
		throw std::runtime_error("cJSON item not a number");
	
	return item->valueint;
}


uint64_t JSON::_get_uint64( const cJSON *item ) {
	if (item->type == cJSON_String)
		try {
			return std::stoull( item->valuestring );
		} catch(...) {
			throw std::runtime_error("cJSON item not a number");
		}

	if (item->type != cJSON_Number)
		throw std::runtime_error("cJSON item not a number");

	return item->valueint;
}


std::string JSON::_get_string( const cJSON *item ) {
	if (item->type != cJSON_String)
		throw std::runtime_error("cJSON item not a string");

	return std::string(item->valuestring);
}


JSON JSON::_get_item( cJSON *item ) {
	if (item->type != cJSON_Object)
		throw std::runtime_error("cJSON item not a object");

	return JSON(item, false);
}


JSON_Array JSON::_get_array( cJSON *item ) {
	if (item->type != cJSON_Array)
		throw std::runtime_error("cJSON item not a array");

	return JSON_Array(item, false);
}


bool JSON::compare_cJSON_children(const cJSON *left, const cJSON *right) {
	return std::string(left->string) < std::string(right->string);
}


void JSON::sort(cJSON *cjson) {
	if (cjson->child == nullptr)
		return;

	std::vector<cJSON *> children;

	cJSON *c = cjson->child;
	while(c) {
		children.push_back(c);
		c = c->next;
	}

	// only actually sort if Object (not Array)
	if (cjson->type == cJSON_Object)
		std::sort( children.begin(), children.end(), JSON::compare_cJSON_children );

	// rebuild
	cJSON *prev = nullptr;

	for(int i=0; i<children.size(); ++i) {
		children[i]->prev = prev;

		if (prev)
			prev->next = children[i];

		prev = children[i];

		if (prev->type == cJSON_Object || prev->type == cJSON_Array)
			JSON::sort(prev);
	}

	if (prev)
		prev->next = nullptr;

	cjson->child = children[0];
}


// public //

bool JSON::exists( const std::string name ) const {
	cJSON *item = cJSON_GetObjectItem( cjson, name.c_str() );

	return item != nullptr;
}


bool JSON::null( const std::string name ) const {
	cJSON *item = cJSON_GetObjectItem( cjson, name.c_str() );

	return item == nullptr || item->type == cJSON_NULL;
}


bool JSON::empty() const {
	return this->cjson->child == nullptr;
}


bool JSON::get_boolean( const std::string name ) const {
	return _get_boolean( _get_item(name) );
}


int JSON::get_number( const std::string name ) const {
	return _get_number( _get_item(name) );
}


double JSON::get_double( const std::string name ) const {
	return  _get_double( _get_item(name) );
}


int64_t JSON::get_int64( const std::string name ) const {
	return  _get_int64( _get_item(name) );
}


uint64_t JSON::get_uint64( const std::string name ) const {
	return  _get_uint64( _get_item(name) );
}


std::string JSON::get_string( const std::string name ) const {
	return  _get_string( _get_item(name) );
}


JSON JSON::get_item( const std::string name ) const {
	return  _get_item( _get_item(name) );
}


JSON_Array JSON::get_array( const std::string name ) const {
	return  _get_array( _get_item(name) );
}


void JSON::add_boolean( const std::string name, const bool value ) {
	cJSON_AddBoolToObject( cjson, name.c_str(), value );
}


void JSON::add_number( const std::string name, const int value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_number( const std::string name, const unsigned int value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_number( const std::string name, const double value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_int( const std::string name, const int value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_uint( const std::string name, const unsigned int value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_double( const std::string name, const double value ) {
	cJSON_AddNumberToObject( cjson, name.c_str(), value );
}


void JSON::add_int64( const std::string name, const int64_t value ) {
	cJSON_AddStringToObject( cjson, name.c_str(), std::to_string(value).c_str() );
}


void JSON::add_uint64( const std::string name, const uint64_t value ) {
	cJSON_AddStringToObject( cjson, name.c_str(), std::to_string(value).c_str() );
}


void JSON::add_string( const std::string name, const std::string value ) {
	cJSON_AddStringToObject( cjson, name.c_str(), value.c_str() );
}


void JSON::add_item( const std::string name, JSON &json ) {
	cJSON_AddItemToObject( cjson, name.c_str(), json.cjson );
	json.is_root = false;
}


void JSON::add_item( const std::string name, JSON_Array &json_array ) {
	cJSON_AddItemToObject( cjson, name.c_str(), json_array.cjson );
	json_array.is_root = false;
}


void JSON::delete_item( const std::string &name ) {
	cJSON_DeleteItemFromObject( cjson, name.c_str() );
}


std::string JSON::to_string() {
	char *json = cJSON_PrintUnformatted(cjson);
	std::string json_string(json);
	free(json);
	
	return json_string;
}


void JSON::sort() {
	JSON::sort(cjson);
}


// JSON_Array //


cJSON *JSON_Array::_get_item( const int index ) const {
	cJSON *item = cJSON_GetArrayItem( cjson, index );
	if (!item)
		throw std::runtime_error("cJSON array index not found");

	return item;
}


JSON_Array::JSON_Array( cJSON *new_cjson, bool new_is_root ) {
	if (new_cjson->type != cJSON_Array)
		throw std::runtime_error("cJSON is not an array");

	cjson = new_cjson;
	is_root = new_is_root;
}


void JSON_Array::push_back( bool value ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateBool(value) );
}


void JSON_Array::push_back( int number ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateNumber(number) );
}


void JSON_Array::push_back( unsigned int number ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateNumber(number) );
}


void JSON_Array::push_back( double number ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateNumber(number) );
}


void JSON_Array::push_back( int64_t number ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateString( std::to_string(number).c_str() ) );
}


void JSON_Array::push_back( uint64_t number ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateString( std::to_string(number).c_str() ) );
}


void JSON_Array::push_back( std::string str ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateString( str.c_str() ) );
}


void JSON_Array::push_back( const char *str ) {
	cJSON_AddItemToArray( cjson, cJSON_CreateString( str ) );
}


void JSON_Array::push_back( JSON &item ) {
	cJSON_AddItemToArray( cjson, item.cjson );
	item.is_root = false;
}


void JSON_Array::push_back( JSON_Array &item_array ) {
	cJSON_AddItemToArray( cjson, item_array.cjson );
	item_array.is_root = false;
}


void JSON_Array::shift() {
	cJSON_DeleteItemFromArray( cjson, 0);
}


int JSON_Array::size() const {
	return cJSON_GetArraySize( cjson );
}


bool JSON_Array::get_boolean( const int index ) const {
	return JSON::_get_boolean( _get_item(index) );
}


int JSON_Array::get_number( const int index ) const {
	return JSON::_get_number( _get_item(index) );
}


double JSON_Array::get_double( const int index ) const {
	return JSON::_get_double( _get_item(index) );
}


int64_t JSON_Array::get_int64( const int index ) const {
	return JSON::_get_int64( _get_item(index) );
}


uint64_t JSON_Array::get_uint64( const int index ) const {
	return JSON::_get_uint64( _get_item(index) );
}


std::string JSON_Array::get_string( const int index ) const {
	return JSON::_get_string( _get_item(index) );
}


JSON JSON_Array::get_item( const int index ) const {
	return JSON::_get_item( _get_item(index) );
}


JSON_Array JSON_Array::get_array( const int index ) const {
	return JSON::_get_array( _get_item(index) );
}


std::string JSON_Array::to_string() {
	char *json = cJSON_PrintUnformatted(cjson);
	std::string json_string(json);
	free(json);

	return json_string;
}


void JSON_Array::clear() {
	while( cJSON_GetArraySize(cjson) )
		cJSON_DeleteItemFromArray(cjson, 0);
}

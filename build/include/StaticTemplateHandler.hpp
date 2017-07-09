#ifndef INCLUDE__STATICTEMPLATEHANDLER_HPP
#define INCLUDE__STATICTEMPLATEHANDLER_HPP

#include "Handler.hpp"
#include "Template.hpp"

#include <list>
#include <string>

typedef Template *(*template_factory_t)(Request *req);

typedef std::pair<std::string, template_factory_t> template_registry_tuple;
typedef std::list<template_registry_tuple> template_registry_t;


template <class T>
Template *template_factory( Request *req ) {
	return new T( req );
};


class StaticTemplateHandler: public Handler {
	private:
		static template_registry_t registry;

	public:
		virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
		
		template <class T>
		static void register_template( std::string url_prefix ) {
			// shove into registry
			registry.push_back( template_registry_tuple( url_prefix, &template_factory<T> ) );
		};
};

#endif

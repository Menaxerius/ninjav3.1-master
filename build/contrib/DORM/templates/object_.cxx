// GENERATED - DO NOT EDIT!

// our class
#include "<%=info.basename%>_.hxx"
#include "<%=info.basename%>.hpp"

// other classes
<% for (auto &navigator : info.navigators) { %>
	#include "<%=navigator.filename%>"
<% } %>


#include <memory>


const std::vector<DORM::Object::Info> <%=info.class_name%>_::column_info = {
	<% for(unsigned int i=0; i < info.columns.size(); ++i) { %>
		<% const auto &column = info.columns[i]; %>
		{
			"<%=column.name%>",
			<%=column.index%>,
			<%= (column.is_key ? "true" : "false") %>,
			<%= (column.not_null ? "true" : "false") %>,
			<%= (column.has_default ? "true" : "false") %>
		}<%= (i != info.columns.size() - 1 ? "," : "") %>
	<% } %>
};


const std::vector< std::function< void(const DORM::Resultset &result, <%=info.class_name%>_ &obj) > > <%=info.class_name%>_::column_resultset_function = {
	<% for(unsigned int i=0; i < info.columns.size(); ++i) { %>
		<% const auto &column = info.columns[i]; %>
		[](const DORM::Resultset &result, <%=info.class_name%>_ &obj){ obj.columns[<%=i%>] = static_cast<<%=column.cxxtype%>>( result.get<%=column.conntype%>(<%=i+1%>) ); }<%= (i != info.columns.size() - 1 ? "," : "") %>
	<% } %>
};


void <%=info.class_name%>_::clear() {
	// explictly [re]set to "empty" values
	<% for(unsigned int i=0; i < info.columns.size(); ++i) { %>
		<% const auto &column = info.columns[i]; %>
		<% if (column.cxxtype == "std::string") { %>
			columns[<%=i%>] = std::string();
		<% } else if (column.cxxtype == "DORM::Timestamp") { %>
			columns[<%=i%>] = DORM::Timestamp();
		<% } else if (column.cxxtype == "bool") { %>
			columns[<%=i%>] = false;
		<% } else { %>
			columns[<%=i%>] = static_cast<<%=column.cxxtype%>>(0);
		<% } %>
	<% } %>

	Object::clear();
}


std::unique_ptr<DORM::Object> <%=info.class_name%>_::make_unique() {
	return std::make_unique<<%=info.class_name%>>();
}


void <%=info.class_name%>_::column_from_resultset( int i, const DORM::Resultset &result ) {
	( column_resultset_function[i] )( result, *this );
}


std::unique_ptr<<%=info.class_name%>> <%=info.class_name%>_::load() {
	DORM::Object::search();
	return result();
}


std::unique_ptr<<%=info.class_name%>> <%=info.class_name%>_::load( <%= info.key_params %> ) {
	<%=info.class_name%> obj;

	<% for( const auto &key : info.keys ) { %>
		obj.<%=key%>( key_<%=key%> );
	<% } %>

	return obj.load();
}


std::unique_ptr<<%=info.class_name%>> <%=info.class_name%>_::load(const DORM::Object &obj) {
	<%=info.class_name%> tmp;
	tmp.copy_columns(obj, true);
	return tmp.load();
}


std::unique_ptr<<%=info.class_name%>> <%=info.class_name%>_::result() {
	if ( !resultset || !resultset->next() ) {
		resultset.reset();
		return std::unique_ptr<<%=info.class_name%>>();
	}

	auto obj = std::make_unique<<%=info.class_name%>>();
	obj->set_from_resultset( *resultset );
	return obj;
}


void <%=info.class_name%>_::search_and_destroy() {
	search();

	while( auto victim = result() )
		victim->delete_obj();
}


std::unique_ptr<<%=info.class_name%>> <%=info.class_name%>_::clone() const {
	auto obj = std::make_unique<<%=info.class_name%>>();

	obj->copy_columns(*this, false);

	for (auto &column : obj->columns)
		column.changed = false;
	return obj;
}


// navigators
<% for (const auto &navigator : info.navigators ) { %>
	std::unique_ptr<<%=navigator.object%>> <%=info.class_name%>_::<%=navigator.function%>() const {
		<% if (navigator.plural) { %>
			auto obj = std::make_unique<<%=navigator.object%>>();
			obj->copy_columns(*this, true);
			return obj;
		<% } else { %>
			return <%=navigator.object%>::load(*this);
		<% } %>
	}
<% } %>

// GENERATED - DO NOT EDIT!

#ifndef DORM__<%=info.class_name%>___HXX
#define DORM__<%=info.class_name%>___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class <%=info.class_name%>;

// other classes
<% for (auto &navigator : info.navigators) { %>
	class <%=navigator.object%>;
<% } %>


class <%=info.class_name%>_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, <%=info.class_name%>_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "<%=info.table_name%>"; };
		virtual const int get_autoinc_index() const { return <%=info.autoinc_index%>; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "<%=info.table_name%>"; };

	public:
		virtual void clear();

		<%=info.class_name%>_() { columns.resize(<%=info.columns.size()%>); clear(); };

		<% for( auto &column : info.columns ) { %>
			inline <%=column.cxxtype%> <%=column.name%>() const { return columns[<%=column.index - 1%>]; };
			inline void <%=column.name%>( const <%=column.cxxtype%> &value) { columns[<%=column.index - 1%>] = value; columns[<%=column.index - 1%>].define(); };

			inline void undef_<%=column.name%>() { columns[<%=column.index - 1%>].undefine(); };
			inline void delete_<%=column.name%>() { columns[<%=column.index - 1%>].remove(); };

			inline bool defined_<%=column.name%>() { return columns[<%=column.index - 1%>].defined; };
			inline bool exists_<%=column.name%>() { return columns[<%=column.index - 1%>].exists; };
		<% } %>

		virtual std::unique_ptr<<%=info.class_name%>> load();
		static std::unique_ptr<<%=info.class_name%>> load(const Object &obj);
		// load using keys
		static std::unique_ptr<<%=info.class_name%>> load( <%=info.key_params%> );

		virtual std::unique_ptr<<%=info.class_name%>> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<<%=info.class_name%>> clone() const;

		// navigators
		<% for (auto &navigator : info.navigators) { %>
			virtual std::unique_ptr<<%=navigator.object%>> <%=navigator.function%>() const;
		<% } %>
};

#endif

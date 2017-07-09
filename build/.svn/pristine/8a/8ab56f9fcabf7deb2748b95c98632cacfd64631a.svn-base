// object .cxx include file template

#include "<%=$prefix%>.hpp"

// class definitions needed by this class
<% foreach my $navigator (@navigators) { %>
  #include "<%=$navigator->{filename}%>.hpp"
<% } %>

// child objects
<% foreach my $child (@children) { %>
  #include "<%=$child->{filename}%>.hpp"
<% } %>



#define FULLTEXT_FIELD(field)

#define SEARCHMOD(name, type)
#define SEARCHMOD_IS_SET(name) _SM_ ## name ## _p
#define SEARCHMOD_VALUE(name) _SM_ ## name
#define SEARCHPREP IO::PreppedSearch *<%=$class%>::search_prep(IDB::Options *options, std::vector<IO::Object *> *additional)
#define SEARCHPREP_SUPER IO::PreppedSearch *ps = this->_search_prep(options, additional);
#define SEARCHPREP_ADD(new_clause) ps->where = ps->where == IDB_NO_WHERE ? new_clause : new IDB::sqlAnd( ps->where, new_clause )
#define SEARCHPREP_END return ps;
#define SEARCHPREP_FULLTEXT(field, value)

#define CHILD_OBJECT(name, navigator)
#define CHILD_OBJECTS(name, navigator)
#define DELETE_CHILDREN _delete_children()

#define NO_NAVIGATOR(field)

#ifdef STATIC
#undef STATIC
#endif
#define STATIC


// SAME ORDER AS .hxx FILE PLEASE!!

const std::string <%=$class%>::_my_table = "<%=$table%>";





// PRIVATE METHODS

// common method to initialize a new object
void <%=$class%>::_init() {
	// init results to null --- should be in superclass really
	results = NULL;

	// initialize search modifiers
	<% foreach my $mod (@search_modifiers) { %>
		_SM_<%=$mod->{name}%>_p = false;
	<% } %>
}


void <%=$class%>::_init_columns() {
	// initialize columns
	<% foreach my $col (@columns) { %>
		_<%=$col->{name}%>_exists = false;
		_<%=$col->{name}%>_defined = false;
		_<%=$col->{name}%>_changed = false;
		_<%=$col->{name}%> = <%=$cpp_default{ $col->{cpp} }%>;
	<% } %>
}


// common method to copy data from an SQL result set into our object
void <%=$class%>::_copy_from_res(IDB::ResultSet *res) {
	try {
		<% for(my $i=0; $i<@columns; $i++) { %>
			<% my $col = $columns[$i] %>
			if ( res->isNull( <%=$i+1%> ) ) {
				this->undef_<%=$col->{name}%>();
			} else {
				this-><%=$col->{name}%>( res->get<%=$col->{conn_type}%>( <%=$i+1%> ) );
			}
			_<%=$col->{name}%>_changed = false;
		<% } %>
	} catch (sql::SQLException &e) {
		std::cerr << "[IDB] " << e.getErrorCode() << ": " << e.what() << std::endl;
		std::cerr << "[IDB] <%=$class%> column index <%=$i%>" << std::endl;
		throw(e);
	}
}


//--- PRIVATE SEARCH METHODS ---//

// method to generate generic search WHERE criteria based on columns with values
IDB::Where *<%=$class%>::_search_prep_columns() {
	IDB::Where					*where_clause = IDB_NO_WHERE;
	IDB::Where					*new_clause;

	<% foreach my $col (@columns) { %>
		if (_<%=$col->{name}%>_exists) {
			if (_<%=$col->{name}%>_defined) {
				<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
					new_clause = new IDB::sqlEq<%=$timestamp_t%>( "<%=$table%>.<%=$col->{name}%>", IDB::Engine::from_unixtime(_<%=$col->{name}%>) );
				<% } else { %>
					new_clause = new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$table%>.<%=$col->{name}%>", _<%=$col->{name}%> );
				<% } %>		
			} else {
				new_clause = new IDB::sqlIsNull( "<%=$table%>.<%=$col->{name}%>" );
			}

			if (where_clause == IDB_NO_WHERE) {
				where_clause = new_clause;
			} else {
				where_clause = new IDB::sqlAnd( where_clause, new_clause );
			}
		}
	<% } %>

	return where_clause;
}


// method to generate generic join ON criteria based on columns
IDB::Where *<%=$class%>::_search_prep_join(std::map<std::string, std::string> *col_to_table) {
	IDB::Where									*on_clause = IDB_NO_WHERE;
	std::map<std::string, std::string>::iterator	it;

	<% foreach my $col (@columns) { %>
		<% next unless grep { $_ eq $col->{name} } @keys %>

		it = col_to_table->find("<%=$col->{name}%>");

		if ( it != col_to_table->end() ) {
			std::string col_name = it->second;
			col_name += ".<%=$col->{name}%>";
			IDB::Where *new_clause = new IDB::sqlEqCol( col_name, "<%=$table%>.<%=$col->{name}%>" );

			if (on_clause == IDB_NO_WHERE) {
				on_clause = new_clause;
			} else {
				on_clause = new IDB::sqlAnd( on_clause, new_clause );
			}
		}
	<% } %>

	return on_clause;
}


// method to do heavy lifting unique to this object for real search_prep() method
IO::PreppedSearch *<%=$class%>::_search_prep(IDB::Options *options, std::vector<IO::Object *> *additional) {
	IO::PreppedSearch							*ps = new IO::PreppedSearch();
	IDB::Tables									*tables = new IDB::Tables("<%=$table%>");
	IDB::Where									*where_clause = IDB_NO_WHERE;
	IDB::Where									*new_clause;
	std::map<std::string, std::string>			col_to_table;

	where_clause = this->_search_prep_columns();

	<% foreach my $col (@columns) { %>
		col_to_table["<%=$col->{name}%>"] = "<%=$table%>";
	<% } %>
	
	if (additional) {
		std::vector<IO::Object *>::iterator	it;
		IDB::Where							*on_clause = IDB_NO_WHERE;
	
		for(it = additional->begin(); it != additional->end(); it++) {
			IO::PreppedSearch *additional_ps = (*it)->search_prep(IDB_NO_OPTIONS, (std::vector<IO::Object *> *) NULL);

			/* join additional using columns from map */
			on_clause = (*it)->_search_prep_join(&col_to_table);
			tables->join("join", (*it)->_table(), on_clause);
			
			/* add additional columns to where */
			new_clause = additional_ps->where;

			if (new_clause != IDB_NO_WHERE) {
				if (where_clause != IDB_NO_WHERE) {
					where_clause = new IDB::sqlAnd( where_clause, new_clause );
				} else {
					where_clause = new_clause;
				}
			}

			// don't delete where clause - we're using it
			additional_ps->where = IDB_NO_WHERE;
			delete additional_ps;
		}
	}

	ps->cols = new std::vector<std::string>();
	/* use distinct results in case additional objects cause multiple rows to be returned for object requested */
	ps->cols->push_back("distinct <%=$table%>.*");

	ps->tables = tables;
	ps->where = where_clause;

	ps->options = new IDB::Options();
	<% while (my ($option, $value) = each %search_options) { %>
		<% if ($option eq 'having') { %>
			ps->options->having = IDB::sqlLiteral("<%=$value%>");
		<% } elsif ($option eq 'limit' || $option eq 'offset') { %>
			ps->options-><%=$option%> = <%=$value%>;
		<% } else { %>
			ps->options-><%=$option%> = "<%=$value%>";
		<% } %>
	<% } %>

	// object's limit & offset applied here but can be overridden by other searchmods
	if ( limit() )
		ps->options->limit = limit();
	if ( offset() )
		ps->options->offset = offset();

	// generic order_by support (can be overridden below)
	if ( !_order_by.empty() )
		ps->options->order_by = _order_by;

	if (options && options != IDB_NO_OPTIONS) {
		if ( options->limit )
			ps->options->limit = options->limit;

		if ( options->offset )
			ps->options->offset = options->offset;

		if ( options->having )
			ps->options->having = options->having;

		if ( !options->group_by.empty() )
			ps->options->group_by = options->group_by;

		if ( !options->order_by.empty() )
			ps->options->order_by = options->order_by;
	}

	return ps;
}



//--- ---/


// method to do heavy lifting unique to this object for real save() method
void <%=$class%>::_save() {
	std::vector<IDB::Where *>	inserts;
	std::vector<IDB::Where *>	updates;

	<% foreach my $col (@columns) { %>
		/* keys dealt with later on */
		<% next if grep { $_ eq $col->{name} } @keys %>

		/* add <%=$col->{name}%> to SQL */

		#ifdef MINIMAL_SAVE
			/* add to UPDATEs if column has changed */
			if (_<%=$col->{name}%>_changed) {
		#endif
				if (_<%=$col->{name}%>_exists) {
					if (_<%=$col->{name}%>_defined) {
						<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
							updates.push_back( new IDB::sqlEq<%=$timestamp_t%>( "<%=$col->{name}%>", IDB::Engine::from_unixtime(_<%=$col->{name}%>) ) );
						<% } else { %>
							updates.push_back( new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$col->{name}%>", _<%=$col->{name}%> ) );
						<% } %>		
					} else {
						<% if ( $col->{not_null} ) { %>
						// column is "not null" and value is undefined so there had better be a default otherwise SQL exception thrown
							updates.push_back( new IDB::sqlEqDefault( "<%=$col->{name}%>" ) );
						<% } else { %>
							updates.push_back( new IDB::sqlEqNull( "<%=$col->{name}%>" ) );
						<% } %>
					}
				}
		#ifdef MINIMAL_SAVE
			}
		#endif

		#ifdef MINIMAL_SAVE
			/* add to INSERTs if column has changed OR is a not-null column with no default (to avoid MySQL error 1364) */
			<% if ($col->{not_null} && !$col->{has_default}) { %>
				if (1) {	/* <%=$col->{name}%> is a "NOT NULL" column with no DEFAULT */
			<% } else { %>
				if ( _<%=$col->{name}%>_changed ) {
			<% } %>
		#endif
				if (_<%=$col->{name}%>_exists) {
					if (_<%=$col->{name}%>_defined) {
						<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
							inserts.push_back( new IDB::sqlEq<%=$timestamp_t%>( "<%=$col->{name}%>", IDB::Engine::from_unixtime(_<%=$col->{name}%>) ) );
						<% } else { %>
							inserts.push_back( new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$col->{name}%>", _<%=$col->{name}%> ) );
						<% } %>
					} else {
						<% if ( $col->{not_null} ) { %>
							// column is "not null" and value is undefined so there had better be a default otherwise SQL exception thrown
							inserts.push_back( new IDB::sqlEqDefault( "<%=$col->{name}%>" ) );
						<% } else { %>
							inserts.push_back( new IDB::sqlEqNull( "<%=$col->{name}%>" ) );
						<% } %>
					}
				}
		#ifdef MINIMAL_SAVE
			}
		#endif
	<% } %>

	/* keys: <%=join(' ', @keys)%> */
	/* this only need to be in the INSERT section */
	<% foreach my $col (@columns) { %>
		<% next unless grep { $_ eq $col->{name} } @keys %>

		if (_<%=$col->{name}%>_exists) {
			if (_<%=$col->{name}%>_defined) {
				/* add <%=$col->{name}%> to updates */
				<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
					inserts.push_back( new IDB::sqlEq<%=$timestamp_t%>( "<%=$col->{name}%>", IDB::Engine::from_unixtime(_<%=$col->{name}%>) ) );
				<% } else { %>
					inserts.push_back( new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$col->{name}%>", _<%=$col->{name}%> ) );
				<% } %>		
			}
		}
	<% } %>

	/* do save */
	try {
		idbe()->writerow("<%=$table%>", inserts, updates);
	} catch (const std::exception &e) {
		/* clean inserts & updates */
		for(auto insert : inserts)
			delete insert;
		for(auto update : updates)
			delete update;

		throw(e);
	}

	/* clean inserts & updates */
	for(auto insert : inserts)
		delete insert;
	for(auto update : updates)
		delete update;

	/* reset changed-ness */
	<% foreach my $col (@columns) { %>
		_<%=$col->{name}%>_changed = false;
	<% } %>	

	/* autoinc support */
	<% if ($autoinc) { %>
		if (!_<%=$autoinc%>_exists || !_<%=$autoinc%>_defined) {
			this-><%=$autoinc%>( idbe()->fetchInt("last_insert_id()", IDB_NO_TABLES, IDB_NO_WHERE, IDB_NO_OPTIONS) );
		}
	<% } %>
};


// special private constructor for internal use that doesn't clean column values
<%=$class%>::<%=$class%>( IDB::ResultSet *res ): IO::Object() {
	this->_init();
	this->_copy_from_res( res );
};





// PUBLIC METHODS

const std::string <%=$class%>::<%=$class%>_column_names[] = {
	<% foreach my $col (@columns) { %>
		"<%=$col->{name}%>",
	<% } %>
};

// our table
std::string <%=$class%>::_table() const {
	return "<%=$table%>";
}


// vector of column names
std::vector<std::string> <%=$class%>::column_name_vector() {
	return std::vector<std::string>( std::begin(<%=$class%>_column_names), std::end(<%=$class%>_column_names) );
}


// generic constructor
<%=$class%>::<%=$class%>() {
	this->_init_columns();
	this->_init();
};


// constructor that uses columns from another object (must be key field in other object)
<%=$class%>::<%=$class%>(const IO::Object *other_obj): IO::Object() {
	this->_init();

	<% foreach my $col (@columns) { %>
		if ( other_obj->is_key_by_name("<%=$col->{name}%>") )
			if ( other_obj->exists_by_name("<%=$col->{name}%>") )
				if ( other_obj->defined_by_name("<%=$col->{name}%>") )
					this-><%=$col->{name}%>( other_obj->get_<%=$col->{conn_type}%>_by_name("<%=$col->{name}%>") );
				else
					this->remove_<%=$col->{name}%>();
			else
				this->remove_<%=$col->{name}%>();
		else
			this->remove_<%=$col->{name}%>();
	<% } %>
};


// destructor
<%=$class%>::~<%=$class%>() {
	if (results)
		delete results;
}

// column accessor methods
<% foreach my $col (@columns) { %>
	void <%=$class%>::<%=$col->{name}%>(<%=$col->{cpp}%> new_value) {
		_<%=$col->{name}%>_exists = true;
		_<%=$col->{name}%>_defined = true;
		_<%=$col->{name}%>_changed = true;
		_<%=$col->{name}%> = new_value;
	}
	
	// inlined: moved to .hxx file
	// <%=$col->{cpp}%> <%=$col->{name}%>()
	 
	void <%=$class%>::remove_<%=$col->{name}%>() {
		_<%=$col->{name}%>_exists = false;
		_<%=$col->{name}%>_defined = false;
		_<%=$col->{name}%>_changed = true;
		_<%=$col->{name}%> = <%=$cpp_default{ $col->{cpp} }%>;
	}
	
	void <%=$class%>::undef_<%=$col->{name}%>() {
		_<%=$col->{name}%>_exists = true;
		_<%=$col->{name}%>_defined = false;
		_<%=$col->{name}%>_changed = true;
		_<%=$col->{name}%> = <%=$cpp_default{ $col->{cpp} }%>;
	}
	
	// inlined:
	// bool <%=$class%>::defined_<%=$col->{name}%>()
	
	// inlined:
	// bool <%=$class%>::exists_<%=$col->{name}%>()
	
	// inlined:
	// bool <%=$class%>::changed_<%=$col->{name}%>()
<% } %>


// navigator methods
<% foreach my $navigator (@navigators) { %>
	<%=$navigator->{object}%> *<%=$class%>::<%=$navigator->{method}%>() const {
		<% my $conn_type = $columns_by_name{ $navigator->{column} }{conn_type} %>
		<% if ($conn_type eq 'Timestamp' || $conn_type eq 'F_Timestamp') { %>
			return <%=$navigator->{object}%>::load( new IDB::sqlEq<%=$timestamp_t%>( "<%=$navigator->{column}%>", $IDB::Engine::from_unixtime( _<%=$navigator->{column}%> ) ) );
		<% } else { %>
			return <%=$navigator->{object}%>::load( new IDB::sqlEq<%=$conn_type%>( "<%=$navigator->{column}%>", _<%=$navigator->{column}%> ) );
		<% } %>
	}
<% } %>

<% foreach my $child (@children) { %>
	<%=$child->{object}%> *<%=$class%>::<%=$child->{navigator}%>() const {
		<% if ($child->{many}) { %>
			return new <%=$child->{object}%>( this );
		<% } else { %>
			return <%=$child->{object}%>::load( this );
		<% } %>
	}
<% } %>


// run-time safe!
bool <%=$class%>::has_field(std::string field) const {
	<% foreach my $col (@columns) { %>
		if (!field.compare("<%=$col->{name}%>")) {
			return true;
		}
	<% } %>

	return false;
}


// run-time safe!
bool <%=$class%>::exists_by_name(std::string field) const {
	<% foreach my $col (@columns) { %>
		if (!field.compare("<%=$col->{name}%>")) {
			return _<%=$col->{name}%>_exists;
		}
	<% } %>

	return false;
}


bool <%=$class%>::defined_by_name(std::string field) const {
	<% foreach my $col (@columns) { %>
		if (!field.compare("<%=$col->{name}%>")) {
			return _<%=$col->{name}%>_defined;
		}
	<% } %>

	return false;
}


bool <%=$class%>::is_key_by_name(std::string field) const {
	<% foreach my $col (@key_columns) { %>
		if (!field.compare("<%=$col->{name}%>"))
			return true;
	<% } %>

	return false;
}	


<% while( my ($conn_type, $cpp) = each %conn_type_to_cpp ) { %>
	<%=$cpp%> <%=$class%>::get_<%=$conn_type%>_by_name(std::string field) const {
		<% foreach my $col (@columns) { %>
			<% next unless $col->{conn_type} eq $conn_type %>

			if (!field.compare("<%=$col->{name}%>")) {
				return _<%=$col->{name}%>;
			}
		<% } %>

		std::cerr << "Unimplemented get<%=$conn_type%>_by_name() call for " << field << std::endl;
		return <%=$cpp_default{ $cpp }%>;
	}
<% } %>


void <%=$class%>::clear() {
	if (results)
		delete results;
		
	_init();
	_init_columns();
}


<%=$class%> *<%=$class%>::clone() const {
	<%=$class%> *c = new <%=$class%>();
	
	<% foreach my $col (@columns) { %>
		if ( _<%=$col->{name}%>_defined )
			c-><%=$col->{name}%>( _<%=$col->{name}%> );
	<% } %>

	return c;
}


<%=$class%> *<%=$class%>::skeleton() const {
	<%=$class%> *skel = new <%=$class%>();
	
	<% foreach my $col (@columns) { %>
		<% next unless grep { $_ eq $col->{name} } @keys %>
		skel-><%=$col->{name}%>( _<%=$col->{name}%> );
	<% } %>

	return skel;
}


<%=$class%> *<%=$class%>::load( <%= join(', ', map { "$_->{cpp} $_->{name}" } @key_columns) %> ) {
	IDB::Where *where_clause = NULL;
	
	<% foreach my $key_col ( @key_columns ) { %>
		 <% if ($key_col == $key_columns[0]) { %>
			<% if ($key_col->{conn_type} eq 'Timestamp' || $key_col->{conn_type} eq 'F_Timestamp') { %>
			 	where_clause = new IDB::sqlEq<%=$timestamp_t%>( "<%=$key_col->{name}%>", IDB::Engine::from_unixtime(<%=$key_col->{name}%>) );
			<% } else { %>
			 	where_clause = new IDB::sqlEq<%=$key_col->{conn_type}%>( "<%=$key_col->{name}%>", <%=$key_col->{name}%> );
			<% } %>
		 <% } else { %>
			<% if ($key_col->{conn_type} eq 'Timestamp' || $key_col->{conn_type} eq 'F_Timestamp') { %>
			 	where_clause = new IDB::sqlAnd( where_clause, new IDB::sqlEq<%=$timestamp_t%>( "<%=$key_col->{name}%>", IDB::Engine::from_unixtime(<%=$key_col->{name}%>) ) );
			<% } else { %>
			 	where_clause = new IDB::sqlAnd( where_clause, new IDB::sqlEq<%=$key_col->{conn_type}%>( "<%=$key_col->{name}%>", <%=$key_col->{name}%> ) );
			<% } %>
		 <% } %>
	<% } %>
	
	return load( where_clause );
};

<%=$class%> *<%=$class%>::load(IDB::Where *where_clause) {
	std::vector<std::string>		cols;
	IDB::Tables						*tables = new IDB::Tables("<%=$table%>");
	IDB::ResultSet					*res;

	cols.push_back("*");

	/* select * from table where_clause */
	res = idbe()->select(&cols, tables, where_clause, IDB_NO_OPTIONS);
	delete tables;
	// Watch out! Don't forget this little line is in place!
	delete where_clause;

	if (res->next()) {
		/* copy columns */
		// use special private constructor
		<%=$class%> *loaded_obj = new <%=$class%>( res );
		delete res;
		return loaded_obj;
	}

	delete res;
	return (<%=$class%> *)0;
};

/* OLD BUT WORKING VERSION:
<%=$class%> *<%=$class%>::load(IO::Object *other_obj) {
	<%=$class%> *search_obj = new <%=$class%>(other_obj);
	<%=$class%> *loaded_obj;

	loaded_obj = search_obj->present();
	delete search_obj;

	return loaded_obj;
};
*/

/* NEW, FASTER VERSION: */
<%=$class%> *<%=$class%>::load(IO::Object *other_obj) {
	// all key columns must exist and be defined (i.e. not undef/null)
	<% foreach my $key_col ( @key_columns ) { %>
		if ( ! other_obj->defined_by_name( "<%=$key_col->{name}%>" ) )
			return NULL;
	<% } %>
	
	return load( <%= join(', ', map { 'other_obj->get_' . $_->{conn_type} . '_by_name( "' . $_->{name} . '" )' } @key_columns) %> );
};


std::vector<<%=$class%> *> <%=$class%>::load_all_vector() {
	<%=$class%> me;
	me.search();
	return me.results_vector();
};


void <%=$class%>::refresh() {
	std::vector<std::string>		cols;
	IDB::Tables						*tables = new IDB::Tables("<%=$table%>");
	IDB::Where						*where_clause = IDB_NO_WHERE;
	IDB::Where						*new_clause;
	IDB::ResultSet					*res;
	
	cols.push_back("*");

	<% foreach my $col (@columns) { %>
		<% next unless grep { $_ eq $col->{name} } @keys %>

		<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
			new_clause = new IDB::sqlEq<%=$timestamp_t%>( "<%=$col->{name}%>", IDB::Engine::from_unixtime( _<%=$col->{name}%> ) );
		<% } else { %>
			new_clause = new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$col->{name}%>", _<%=$col->{name}%> );
		<% } %>

		if (where_clause == IDB_NO_WHERE) {
			where_clause = new_clause;
		} else {
			where_clause = new IDB::sqlAnd( where_clause, new_clause );
		}
	<% } %>

	res = idbe()->select(&cols, tables, where_clause, IDB_NO_OPTIONS);
	delete tables;
	delete where_clause;

	if (res->next()) {
		this->_copy_from_res(res);
	}

	delete res;
};

<% if (@children) { %>
	// heavy lifting for deleting children
	void <%=$class%>::_delete_children() {
		<% for(my $i=0; $i<@children; $i++) { %>
			<% my $child = $children[$i] %>
			<%=$child->{object}%> *children<%=$i%> = this-><%=$child->{navigator}%>();
			<% if ($child->{many}) { %>
				children<%=$i%>->search_and_destroy();
			<% } else { %>
				children<%=$i%>->delete_obj();
			<% } %>
			delete children<%=$i%>;
		<% } %>
	}
<% } %>

// heavy lifting for delete_obj()
void <%=$class%>::_delete_obj() {
	<% if (@children && !$delete_obj) { %>
		// no class-special delete_obj() method so we need to delete children ourself
		_delete_children();
	<% } %>
	
	IDB::Where			*where_clause = IDB_NO_WHERE;
	IDB::Where			*new_clause;

	<% foreach my $col (@columns) { %>
		<% next unless grep { $_ eq $col->{name} } @keys %>

		// ignore undefined keys - allows a more crude version of search-and-destroy 
		// where objects DON'T have ->delete_obj() called on them but are simply
		// deleted by database
		
		if ( _<%=$col->{name}%>_defined ) {
			<% if ($col->{conn_type} eq 'Timestamp' || $col->{conn_type} eq 'F_Timestamp') { %>
				new_clause = new IDB::sqlEq<%=$timestamp_t%>( "<%=$col->{name}%>", IDB::Engine::from_unixtime( _<%=$col->{name}%> ) );
			<% } else { %>
				new_clause = new IDB::sqlEq<%=$col->{conn_type}%>( "<%=$col->{name}%>", _<%=$col->{name}%> );
			<% } %>
	
			if (where_clause == IDB_NO_WHERE) {
				where_clause = new_clause;
			} else {
				where_clause = new IDB::sqlAnd( where_clause, new_clause );
			}
		}
	<% } %>

	idbe()->deleterow("<%=$table%>", where_clause);
	delete where_clause;
};

<%=$class%> *<%=$class%>::present() { return this->present(IDB_NO_OPTIONS, (std::vector<IO::Object *> *) NULL ); }

<%=$class%> *<%=$class%>::present(IDB::Options *options, std::vector<IO::Object *> *additional) {
	IO::PreppedSearch		*ps;
	IDB::ResultSet			*res;
	IDB::Options			*tmpOptions = IDB_NO_OPTIONS;

	ps = this->search_prep(options, additional);

	if (!ps->options || ps->options == IDB_NO_OPTIONS) {
		tmpOptions = new IDB::Options();
		ps->options = tmpOptions;
	}

	ps->options->limit = 1;

	res = idbe()->select(ps->cols, ps->tables, ps->where, ps->options);

	if (tmpOptions) {
		delete tmpOptions;
	}

	delete ps;

	if (res->next()) {
		<%=$class%> *loaded_obj = new <%=$class%>( res );
		delete res;
		return loaded_obj;
	}

	delete res;
	return (<%=$class%> *)0;
}


<%=$class%> *<%=$class%>::present_or_self() { 
	<%=$class%> *found = this->present(IDB_NO_OPTIONS, 0);
	
	if (found)
		return found;
	else
		return this;
}


<%=$class%> *<%=$class%>::present_or_clone() { 
	<%=$class%> *found = this->present(IDB_NO_OPTIONS, 0);
	
	if (found)
		return found;
	else
		return this->clone();
}


<%=$class%> *<%=$class%>::result() {
	if (results) {
		if (results->next()) {
			return new <%=$class%>( results );
		}
		
		delete results;
		results = NULL;
	}
	
	return (<%=$class%> *)0;
}


std::unique_ptr<<%=$class%>> <%=$class%>::scoped_result() {
	return std::unique_ptr<<%=$class%>>( this->result() );
}


std::vector<<%=$class%> *> <%=$class%>::results_vector() {
	std::vector<<%=$class%> *> output_vector;

	while( <%=$class%> *vector_entry = result() ) {
		output_vector.push_back( vector_entry );	
	}

	return output_vector;
}

		
// search modifiers
<% foreach my $mod (@search_modifiers) { %>
	void <%=$class%>::<%=$mod->{name}%>(<%=$mod->{type}%> v) { _SM_<%=$mod->{name}%>_p = true; _SM_<%=$mod->{name}%> = v; }
<% } %>


// order by
void <%=$class%>::order_by( <%=$class%>_columns_t col, order_direction_t dir ) {
	_order_by = <%=$class%>_column_names[col] + " " + order_direction_names[dir];
}

// check database shape matches code
bool <%=$class%>::check_db_shape() {
	bool shape_OK = true;

	std::vector<std::string> cols;
	cols.push_back("*");

	IDB::Tables *tables = new IDB::Tables( "INFORMATION_SCHEMA.COLUMNS" );

	IDB::Where *where_schema = new IDB::sqlEqCol( "table_schema", "database()" );
	IDB::Where *where_table = new IDB::sqlEqString( "table_name", "<%=$table%>" );
	
	IDB::Where *where = new IDB::sqlAnd( where_schema, where_table );

	IDB::ResultSet *res = idbe()->select(&cols, tables, where, IDB_NO_OPTIONS);

	<% for(my $i=0; $i<@columns; $i++) { %>
		<% my $col = $columns[$i] %>

		if ( res->next() ) {
			if ( res->getString( 4 ) != "<%=$col->{name}%>" ) {
				std::cerr << "<%=$table%> column #<%=$i+1%> should be '<%=$col->{name}%>' but mySQL returns '" << res->getString(4) << "'" << std::endl;
				shape_OK = false;
			}
		} else {
			std::cerr << "<%=$table%> column #<%=$i+1%> should be '<%=$col->{name}%>' but mySQL ran out of columns!" << std::endl;
			shape_OK = false;
		}
	<% } %>

	return shape_OK;
}


// lock this database record
void <%=$class%>::lock_record() {
	// SELECT true FROM <table> WHERE <defined keys> FOR UPDATE
	std::vector<std::string> cols;
	cols.push_back("true");

	IDB::Tables *tables = new IDB::Tables("<%=$table%>");

	IDB::Where *where_clause = nullptr;
	
	<% foreach my $key_col ( @key_columns ) { %>
		if ( _<%=$key_col->{name}%>_defined ) {
			<% if ($key_col->{conn_type} eq 'Timestamp' || $key_col->{conn_type} eq 'F_Timestamp') { %>
			 	IDB::Where *key_clause = new IDB::sqlEq<%=$timestamp_t%>( "<%=$key_col->{name}%>", IDB::Engine::from_unixtime(_<%=$key_col->{name}%>) );
			<% } else { %>
			 	IDB::Where *key_clause = new IDB::sqlEq<%=$key_col->{conn_type}%>( "<%=$key_col->{name}%>", _<%=$key_col->{name}%> );
			<% } %>

			if (where_clause == nullptr)
				where_clause = key_clause;
			else
				where_clause = new IDB::sqlAnd( where_clause, key_clause );
		}
	<% } %>

	IDB::Options *options = new IDB::Options;
	options->for_update = true;
	
	IDB::ResultSet *res = idbe()->select(&cols, tables, where_clause, options);
	
	delete res;

	delete options;
	delete where_clause;
	delete tables;
}


// lock table to prevent updates
void <%=$class%>::lock_table() {
	// SELECT true from <table> LOCK IN SHARE MODE
	std::vector<std::string> cols;
	cols.push_back("true");

	IDB::Tables *tables = new IDB::Tables("<%=$table%>");

	IDB::Options *options = new IDB::Options;
	options->lock_in_share_mode = true;
	
	IDB::ResultSet *res = idbe()->select(&cols, tables, NULL, options);
	
	delete res;

	delete options;
	delete tables;
}

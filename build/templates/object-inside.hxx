// object "inside" .hxx template

	private:
		// search modifier variables
		<% foreach my $mod (@search_modifiers) { %>
			bool _SM_<%=$mod->{name}%>_p;
			<%=$mod->{type}%> _SM_<%=$mod->{name}%>;
		<% } %>

		// our columns
		<% foreach my $col (@columns) { %>
			<%=$col->{cpp}%> _<%=$col->{name}%>;
			bool _<%=$col->{name}%>_exists;
			bool _<%=$col->{name}%>_defined;
			bool _<%=$col->{name}%>_changed;
		<% } %>

		// generic order_by support
		std::string _order_by;
		
		// handy?
		static const std::string _my_table;
	
	// methods
	
	private:
		// common method to initialize a new object
		void _init();

		// common method to initialize column values for an object
		void _init_columns();

		// common method to copy data from an SQL result set into our object
		void _copy_from_res(IDB::ResultSet *res);

		// method to generate generic search WHERE criteria based on columns with values
		IDB::Where *_search_prep_columns();
		
		// method to generate generic join ON criteria based on columns
		IDB::Where *_search_prep_join(std::map<std::string, std::string> *col_to_table);
		
		// method to do heavy lifting unique to this object for real search_prep() method
		IO::PreppedSearch *_search_prep(IDB::Options *options, std::vector<IO::Object *> *additional);

		// method to do heavy lifting unique to this object for real save() method
		void _save();
		
		// method to do heavy lifting unique to this object for real delete_obj() method
		void _delete_obj();
		
		<% if (@children) { %>
			// method to do heavy lifting of deleting child objects
			void _delete_children();
		<% } %>

		// special constructor for internal use that doesn't clean column values
		<%=$class%>( IDB::ResultSet *res );

	public:
		// our enum of columns
		typedef enum {
			<% foreach my $col (@columns) { %>
				COL_<%=$col->{name}%>,
			<% } %>
		} <%=$class%>_columns_t;

		static const std::string <%=$class%>_column_names[];
	
		// our table
		std::string _table() const;
		
		// vector of column names
		std::vector<std::string> column_name_vector();
	
		// generic constructor
		<%=$class%>();
		
		// constructor that uses columns from another object
		<%=$class%>(const IO::Object *other_obj);

		// destructor
		virtual ~<%=$class%>();


		// column accessor methods
		<% foreach my $col (@columns) { %>
			void <%=$col->{name}%>(<%=$col->{cpp}%> new_value);

			inline <%=$col->{cpp}%> <%=$col->{name}%>() const {
				return _<%=$col->{name}%>;
			};

			void remove_<%=$col->{name}%>();

			void undef_<%=$col->{name}%>();

			inline bool defined_<%=$col->{name}%>() const {
				return _<%=$col->{name}%>_defined;
			};

			inline bool exists_<%=$col->{name}%>() const {
				return _<%=$col->{name}%>_exists;
			};

			inline bool changed_<%=$col->{name}%>() const {
				return _<%=$col->{name}%>_changed;
			};
		<% } %>

		
		// navigator methods
		// these can't be named the same as the object they navigate to due to C++ conflict
		<% foreach my $navigator (@navigators) { %>
			<%=$navigator->{object}%> *<%=$navigator->{method}%>() const;
		<% } %>

		// children navigators
		<% foreach my $child (@children) { %>
			<%=$child->{object}%> *<%=$child->{navigator}%>() const;
		<% } %>
		

		// generic column testing by name
		bool has_field(std::string field) const;
		bool exists_by_name(std::string field) const;
		bool defined_by_name(std::string field) const;
		bool is_key_by_name(std::string field) const;


		// get specific column typed value by name
		<% while( my ($conn_type, $cpp) = each %conn_type_to_cpp ) { %>
			<%=$cpp%> get_<%=$conn_type%>_by_name(std::string field) const;
		<% } %>

		// method to clear all fields of object
		void clear();
			
		// method to generate a clone of object
		<%=$class%> *clone() const;

		// method to generate empty object but with keys filled in
		// (like a clone but only keys)
		<%=$class%> *skeleton() const;

		// method to load this object via explicit keys
		static <%=$class%> *load( <%= join(', ', map { "$_->{cpp} $_->{name}" } @key_columns) %> );

		// method to load this object via a WHERE clause
		static <%=$class%> *load(IDB::Where *where_clause);
		
		// method to load this object via key values from columns in another object
		static <%=$class%> *load(IO::Object *other_obj);

		// method to load ALL objects from database as a vector
		static std::vector<<%=$class%> *> load_all_vector();

		// reload this object from database based on keys
		void refresh();

		// "present" is like search(...) followed by result() - returns first matching record (if any)
		<%=$class%> *present();
		<%=$class%> *present(IDB::Options *options, std::vector<IO::Object *> *additional);
		// present_or_self either finds a match or just returns the calling object
		<%=$class%> *present_or_self();
		// present_or_clone either finds a match or just returns a clone calling object
		<%=$class%> *present_or_clone();

		// return first/next matching record or null if none left
		<%=$class%> *result();

		// XXX experimental version of result() above but returns a unique_ptr which only lives until end of scope
		// could be useful in cases like:
		// while( auto thing = searched_things->result() ) { ... }
		std::unique_ptr<<%=$class%>> scoped_result();

		// return a vector of all matching records
		std::vector<<%=$class%> *> results_vector();
		
		// search modifiers
		<% foreach my $mod (@search_modifiers) { %>
			void <%=$mod->{name}%>(<%=$mod->{type}%> v);
		<% } %>
		
		// ordering modifiers
		void order_by( <%=$class%>_columns_t col, order_direction_t dir );
		
		<% if ($search_prep) { %>
			IO::PreppedSearch *search_prep(IDB::Options *options, std::vector<IO::Object *> *additional);
		<% } %>
				
		// other methods
		<% foreach my $method (@methods) { %>
			<%=$method->{returns}%> <%=$method->{name}%>(<%=$method->{args}%>);
		<% } %>
		
		static bool check_db_shape();
		
		// lock this record
		void lock_record();
		
		// lock table in read-only
		static void lock_table();

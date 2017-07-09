#ifndef DORM__INCLUDE__OBJECT_HPP
#define DORM__INCLUDE__OBJECT_HPP

#include "SPC.hpp"
#include "Resultset.hpp"
#include "Column.hpp"

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <initializer_list>


namespace DORM {

	class Where;
	class Query;

	class Object {
		protected:
			class Info {
				public:
					std::string		name;
					int				index;
					bool			is_key;
					bool			not_null;
					bool			has_default;
			};

			std::vector<Column>			columns;
			std::unique_ptr<Resultset>	resultset;

			virtual const std::vector<Info> &get_column_info() const =0;

			virtual const std::string get_table_name() const =0;
			virtual const int get_autoinc_index() const { return 0; };

			virtual void search_prep_columns(Query &query) const;

			virtual std::unique_ptr<Object> make_unique() =0;	// actually returns std::unique_ptr<derived Object>
			virtual void set_from_resultset(const Resultset &result);
			virtual void column_from_resultset(int i, const Resultset &result) =0;

		public:
			enum LOCK_MODE { SHARE, EXCLUSIVE };

			uint64_t		limit = 0;
			uint64_t		offset = 0;

			Object() {};
			Object(Object &&) =delete;			// might be safe to allow move-constructor

			Object(const Object &) =delete;		// really likely to be a bad idea - use clone() instead

			virtual ~Object() {};

			void copy_columns( const Object &other_obj, bool only_keys = false );

			virtual void clear();

			// virtual unique_ptr<derived Object> load()					<-- used to be present()
			// static unique_ptr<derived Object> load( { key values } )
			// static unique_ptr<derived Object> load( const Where & )		<-- used by navigators to parent object (e.g. UserComments to Users via userID)
			// static unique_ptr<derived Object> load( const Object & )		<-- need all our key columns from other Object, used by CHILD_OBJECT navigators

			virtual void save();

			virtual uint64_t search( std::initializer_list< std::reference_wrapper<const Object> > objs );
			virtual uint64_t search() { std::initializer_list< std::reference_wrapper<const Object> > objs = {}; return search(objs); };

			virtual void delete_obj();
			virtual void search_and_destroy() =0;	// has to be generated otherwise tries to search base "Object"s instead of derived objects

			virtual void search_prep( Query &query ) const {};

			// virtual unique_ptr<derived Object> result()		<-- generated and not declared here due to derived return type

			virtual void refresh();
			virtual bool present();
			virtual uint64_t count();

			virtual bool lock_record( LOCK_MODE mode = EXCLUSIVE );
			virtual uint64_t lock_records( LOCK_MODE mode = EXCLUSIVE );

			// clone() copies all columns but the clone's columns have "changed" reset to false
			// virtual unique_ptr<derived Object> clone() const		<-- generated and not declared here due to derived return type
	};

}

#endif

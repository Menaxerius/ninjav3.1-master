#include "Object.hpp"
#include "DB.hpp"
#include "Query.hpp"
#include "Resultset.hpp"

#include "sql/sqlEq.hpp"
#include "sql/sqlAnd.hpp"
#include "sql/sqlIsNull.hpp"

#include <iostream>


namespace DORM {

	void Object::search_prep_columns(Query &query) const {
		const auto &column_info = get_column_info();

		std::vector< SPC<Where> > where_clauses;

		for(const auto &info : column_info) {
			const auto &column = columns[ info.index - 1 ];

			if ( !column.exists )
				continue;

			if ( column.defined )
				where_clauses.push_back( column.column_eq(info.name) );
			else
				where_clauses.push_back( sqlIsNull(info.name).make_shared() );
		}

		if ( !where_clauses.empty() )
			query.where = sqlAnd(where_clauses).make_shared();
	}


	void Object::set_from_resultset(const Resultset &result) {
		const auto &column_info = get_column_info();
		const int n_columns = column_info.size();

		int i;

		try {
			for(i=0; i<n_columns; ++i) {
				// check for NULLs first
				if ( result.isNull(i+1) ) {
					columns[i].defined = false;
				} else {
					column_from_resultset(i, result);
					columns[i].defined = true;
				}

				columns[i].exists = true;
			}
		} catch (sql::SQLException &e) {
			std::cerr << "[DORM] " << e.getErrorCode() << ": " << e.what() << std::endl;
			std::cerr << "[DORM] " << get_table_name() << " column index " << i+1 << std::endl;
			throw;
		}
	}


	// --- public --- //

	void Object::copy_columns(const Object &other_obj, bool only_keys ) {
		const auto &our_column_info = get_column_info();
		const int n_our_columns = our_column_info.size();

		const auto &other_column_info = other_obj.get_column_info();
		const int n_other_columns = other_column_info.size();

		for(int our_col_idx=0; our_col_idx<n_our_columns; ++our_col_idx) {
			const auto &our_info = our_column_info[our_col_idx];

			if ( only_keys && !our_info.is_key )
				continue;

			for(int other_col_idx=0; other_col_idx<n_other_columns; ++other_col_idx) {
				const auto &other_info = other_column_info[other_col_idx];
				const auto &other_column = other_obj.columns[ other_info.index - 1 ];

				// XXX maybe this should copy "undefined" columns?
				if ( !other_column.exists || !other_column.defined || other_info.name != our_info.name )
					continue;

				columns[our_col_idx] = other_column;
				columns[our_col_idx].defined = columns[our_col_idx].exists = true;
			}
		}
	}


	void Object::clear() {
		// reset all columns back to initial values
		// this is done by subclass

		// reset all column states to unchanged, undefined and nonexistent
		for (auto &column : columns)
			column.changed = column.defined = column.exists = false;
	}


	void Object::save() {
		std::vector< SPC<Where> > updates;
		std::vector< SPC<Where> > inserts;

		const auto &column_info = get_column_info();

		/*
		 *	foreach non-key column:
		 * 		if column exists and has changed (MINIMAL_SAVE)
		 * 			if column defined
		 * 				add column to UPDATEs using specific sqlEq<> (take care with timestamps?)
		 * 			else
		 * 				if column declared as "not null"
		 * 					add column to UPDATEs using sqlEq<Default> (to set it to DEFAULT)
		 * 				else
		 * 					add column to UPDATEs using sqlEq<Null> (to set it to NULL)
		 *
		 * 		also do the same with INSERT if column has changed OR is declared NOT NULL with no DEFAULT (to avoid MySQL error 1364)
		 */
		for(const auto &info : column_info) {
			if ( info.is_key )
				continue;

			#ifdef DORM_OBJECT_DEBUG
				std::cout << "[DORM] Non-key column: " << info.name << std::endl;
			#endif

			const auto &column = columns[ info.index - 1 ];

			if ( column.exists && column.changed ) {
				if ( column.defined )
					updates.push_back( column.column_eq(info.name) );
				else if ( info.not_null )
					updates.push_back( sqlEq<Default>(info.name).make_shared() );
				else
					updates.push_back( sqlEq<Null>(info.name).make_shared() );
			}

			if ( (info.not_null && !info.has_default) || column.changed ) {
				if ( info.not_null && !info.has_default && !column.defined )
					std::cerr << "[DORM] Following save() likely to fail - considering making '" << info.name << "' column NULL or add DEFAULT" << std::endl;

				if ( column.defined )
					inserts.push_back( column.column_eq(info.name) );
				else if ( info.not_null )
					inserts.push_back( sqlEq<Default>(info.name).make_shared() );	// could still cause mySQL error 1364
				else
					inserts.push_back( sqlEq<Null>(info.name).make_shared() );
			}
		}

		// add all key columns to INSERTs, regardless of whether they've changed
		for(const auto &info : column_info) {
			if ( !info.is_key )
				continue;

			#ifdef DORM_OBJECT_DEBUG
				std::cout << "[DORM] Key column: " << info.name << std::endl;
			#endif

			const auto &column = columns[ info.index - 1 ];

			if ( column.exists && column.defined )
				inserts.push_back( column.column_eq(info.name) );
		}

		// writerow was in try..catch block -- WHY?
		DB::writerow( get_table_name(), inserts, updates);

		// grab last_insert_id() if table has AUTO_INCREMENT column
		const int autoinc_index = get_autoinc_index();
		if (autoinc_index) {
			auto &column = columns[ autoinc_index - 1 ];

			if ( !column.exists || !column.defined ) {
				Query query;
				query.cols.push_back("last_insert_id()");

				const uint64_t autoinc_value = DB::fetch_uint64(query);		// what happens if this throws but the previous writerow() did not?

				column = autoinc_value;
				column.exists = column.defined = true;
			}
		}

		// reset column changed flags
		for(auto &column : columns)
			column.changed = false;
	}


	uint64_t Object::search( std::initializer_list< std::reference_wrapper<const Object> > objs ) {
		const std::string table_name = get_table_name();

		Query query;
		query.cols.push_back("*");
		query.tables = Tables( table_name );

		// copy any LIMIT / OFFSET from us first, but can be overridden by search_prep()
		query.limit = limit;
		query.offset = offset;

		// convert columns
		search_prep_columns(query);

		// join additional objects
		if ( objs.size() > 0 ) {
			std::map<std::string, std::string> table_by_column;

			const auto &column_info = get_column_info();

			for(const auto &info : column_info)
				table_by_column[ info.name ] = table_name;

			for( const auto &obj_ref : objs ) {
				const auto &obj = obj_ref.get();

				Query join_query;

				obj.search_prep_columns( join_query );

				obj.search_prep(query);

				const auto &obj_column_info = obj.get_column_info();
				const std::string obj_table_name = obj.get_table_name();

				for( const auto &obj_info : obj_column_info ) {
					const auto &map_it = table_by_column.find( obj_info.name );

					if ( map_it != table_by_column.end() ) {
						const std::string &obj_col_name = obj_table_name + "." + obj_info.name;

						join_query.and_where( sqlEq<ColName>( obj_col_name, table_name + "." + obj_info.name ) );
					} else {
						// update map of columns to table with new, previously unknown, column
						table_by_column[ obj_info.name ] = obj_table_name;
					}
				}

				// JOIN table
				query.tables.join( "JOIN", obj_table_name, join_query.where );
			}
		}

		search_prep(query);

		// mySQL-only
		query.cols[0] = "SQL_CALC_FOUND_ROWS " + query.cols[0];
		resultset.reset( DB::select(query) );

		Query found_rows_query;
		found_rows_query.cols.push_back("found_rows()");
		const uint64_t found_rows = DB::fetch_uint64(found_rows_query);

		return found_rows;
	}


	void Object::delete_obj() {
		const std::string table_name = get_table_name();

		const auto &column_info = get_column_info();

		std::vector< SPC<Where> > where_clauses;

		for(const auto &info : column_info) {
			const auto &column = columns[ info.index - 1 ];

			if ( !column.exists || !column.defined )
				continue;

			where_clauses.push_back( column.column_eq(info.name) );
		}

		sqlAnd where(where_clauses);

		DB::deleterow(table_name, where);
	}


	void Object::refresh() {
		// we only need keys to reload the object
		Query query;
		query.cols.push_back("*");
		query.tables = Tables( get_table_name() );
		query.limit = 1;

		const auto &column_info = get_column_info();

		std::vector< SPC<Where> > where_clauses;

		for(const auto &info : column_info) {
			if ( !info.is_key )
				continue;

			const auto &column = columns[ info.index - 1 ];

			if ( column.exists &&  column.defined )
				where_clauses.push_back( column.column_eq(info.name) );
		}

		if ( !where_clauses.empty() )
			query.where = sqlAnd(where_clauses).make_shared();

		std::unique_ptr<Resultset> results( DB::select(query) );

		if ( !results || !results->next() ) {
			sql::SQLException e("Row not found for refresh()");
			std::cerr << "[DORM] " << e.getErrorCode() << ": " << e.what() << std::endl;
			std::cerr << "[DORM] " << get_table_name() << std::endl;
			throw;
		}

		set_from_resultset( *results );

		// reset state flags
		for(auto &column : columns)
			column.changed = false;
	}


	bool Object::present() {
		Query query;
		query.cols.push_back("*");
		query.tables = Tables( get_table_name() );

		// convert columns
		search_prep_columns(query);

		search_prep(query);

		query.cols.assign( {"true"} );
		query.limit = 1;

		std::unique_ptr<Resultset> results( DB::select(query) );

		if ( results && results->next() )
			return results->getBoolean(1);

		return false;
	}


	uint64_t Object::count() {
		Query query;
		query.cols.push_back("*");
		query.tables = Tables( get_table_name() );

		// convert columns
		search_prep_columns(query);

		search_prep(query);

		query.limit = 1;

		/* we do a full search then use SQL_CALC_FOUND_ROWS ? */
		/* why not use count(*) ? */

		// mySQL-only
		query.cols[0] = "SQL_CALC_FOUND_ROWS " + query.cols[0];
		std::unique_ptr<Resultset> results( DB::select(query) );

		Query found_rows_query;
		found_rows_query.cols.push_back("found_rows()");
		const uint64_t found_rows = DB::fetch_uint64(found_rows_query);

		return found_rows;
	}


	bool Object::lock_record( LOCK_MODE lock_mode ) {
		// SELECT true FROM <table> WHERE <defined keys> FOR UPDATE
		Query query;
		query.cols.push_back("SQL_CALC_FOUND_ROWS true");
		query.tables = Tables( get_table_name() );
		query.limit = 1;

		if (lock_mode == EXCLUSIVE)
			query.for_update = true;
		else
			query.lock_in_share_mode = true;

		const auto &column_info = get_column_info();

		std::vector< SPC<Where> > where_clauses;

		for(const auto &info : column_info) {
			if ( !info.is_key )
				continue;

			const auto &column = columns[ info.index - 1 ];

			if ( column.exists &&  column.defined )
				where_clauses.push_back( column.column_eq(info.name) );
		}

		if ( !where_clauses.empty() )
			query.where = sqlAnd(where_clauses).make_shared();

		std::unique_ptr<Resultset> results( DB::select(query) );

		Query found_rows_query;
		found_rows_query.cols.push_back("found_rows()");
		const uint64_t found_rows = DB::fetch_uint64(found_rows_query);

		return found_rows == 1;
	}


	uint64_t Object::lock_records( LOCK_MODE lock_mode ) {
		// SELECT true FROM <tables> WHERE <...> FOR UPDATE
		Query query;
		query.cols.push_back("*");
		query.tables = Tables( get_table_name() );

		// convert columns
		search_prep_columns(query);

		search_prep(query);

		// rewrite slightly
		query.cols.assign( {"SQL_CALC_FOUND_ROWS true"} );

		if (lock_mode == EXCLUSIVE)
			query.for_update = true;
		else
			query.lock_in_share_mode = true;

		std::unique_ptr<Resultset> results( DB::select(query) );

		Query found_rows_query;
		found_rows_query.cols.push_back("found_rows()");
		const uint64_t found_rows = DB::fetch_uint64(found_rows_query);

		return found_rows;
	}

}

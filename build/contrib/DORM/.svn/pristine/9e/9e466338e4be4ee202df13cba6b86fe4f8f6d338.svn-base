#ifndef DORM__INCLUDE__Tables__HPP
#define DORM__INCLUDE__Tables__HPP

#include "SPC.hpp"

#include <cppconn/prepared_statement.h>
#include <vector>
#include <string>


namespace DORM {

	class Table;
	class Where;
	class TableOrSubquery;


	class Tables {
		private:
			class TableJoin {
				public:
					std::string								join;
					SPC<TableOrSubquery>					table;
					SPC<Where>								on_clause;

					TableJoin( std::string join_type, std::string new_table, const Where &join_on_clause );
					TableJoin( std::string join_type, const TableOrSubquery &new_table, const Where &join_on_clause );

					TableJoin( std::string join_type, std::string new_table, SPC<Where> join_on_clause );
					TableJoin( std::string join_type, const TableOrSubquery &new_table, SPC<Where> join_on_clause );

					std::string to_string() const;
					void bind( sql::PreparedStatement &pstmt, unsigned int &bind_offset ) const;
			};

			SPC<TableOrSubquery>						initial_table;
			std::vector<TableJoin>						table_joins;

		public:
			Tables() {};
			Tables( std::string table );
			Tables( const TableOrSubquery &table );

			void join( std::string join_type, std::string new_table, const Where &join_on_clause );
			void join( std::string join_type, const TableOrSubquery &new_table, const Where &join_on_clause );

			void join( std::string join_type, std::string new_table, SPC<Where> join_on_clause );
			void join( std::string join_type, const TableOrSubquery &new_table, SPC<Where> join_on_clause );

			std::string to_string() const;
			void bind( sql::PreparedStatement &pstmt, unsigned int &bind_offset ) const;
	};

}

#endif

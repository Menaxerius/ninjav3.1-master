#ifndef DORM__INCLUDE__TABLEORSUBQUERY__HPP
#define DORM__INCLUDE__TABLEORSUBQUERY__HPP

#include "SPC.hpp"

#include <cppconn/prepared_statement.h>
#include <string>


namespace DORM {

	class Query;


	class TableOrSubquery {
		public:
			virtual SPC<TableOrSubquery> make_shared() const =0;

			virtual std::string to_string() const =0;
			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const =0;
	};
	

	class Table: public TableOrSubquery {
		private:
			std::string						table_name;
	
		public:
			Table(std::string table): table_name(table) {};

			virtual SPC<TableOrSubquery> make_shared() const;

			virtual std::string to_string() const { return table_name; };
			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};
	};
	

	class Subquery: public TableOrSubquery {
		private:
			std::shared_ptr<const Query>	subquery;
			std::string						subquery_alias;
			
		public:
			// NOTE: We're using a shared_ptr to a copy of the passed Query to remove header includes
			Subquery( const Query &query, std::string alias );

			virtual SPC<TableOrSubquery> make_shared() const;

			virtual std::string to_string() const;
			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const;
	};
}

#endif

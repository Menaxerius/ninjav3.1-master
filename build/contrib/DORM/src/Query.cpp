#include "Query.hpp"

#include "sql/sqlAnd.hpp"


namespace DORM {

	SPC<Query> Query::make_shared() const {
		return std::make_shared<const Query>(*this);
	}


	std::string Query::to_string() const {
		std::string sql = "SELECT ";

		for(auto &col : cols)
			sql += col + ", ";
		sql.pop_back();
		sql.pop_back();

		sql += tables.to_string();

		if (where)
			sql += " WHERE " + where->to_string();

		if ( !group_by.empty() )
			sql += " GROUP BY " + group_by;

		if (having)
			sql += " HAVING " + having->to_string();

		if ( !order_by.empty() )
			sql += " ORDER BY " + order_by;

		if (limit > 0)
			sql += " LIMIT " + std::to_string(limit);

		if (offset > 0)
			sql += " OFFSET " + std::to_string(offset);

		if (lock_in_share_mode)
			sql += " LOCK IN SHARE MODE ";

		if (for_update)
			sql += " FOR UPDATE ";

		return sql;
	}


	void Query::bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {
		tables.bind(pstmt, bind_offset);

		if (where)
			where->bind(pstmt, bind_offset);

		if (having)
			having->bind(pstmt, bind_offset);
	}


	void Query::and_where( const Where &some_where ) {
		if (where)
			where = sqlAnd( where, some_where.make_shared() ).make_shared();
		else
			where = some_where.make_shared();
	}

}

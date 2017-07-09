#include "Tables.hpp"

#include "Where.hpp"
#include "TableOrSubquery.hpp"


namespace DORM {

	Tables::TableJoin::TableJoin( std::string join_type, std::string new_table, const Where &join_on_clause ) {
		join = join_type;
		table = std::make_shared<Table>(new_table);
		on_clause = join_on_clause.make_shared();
	}


	Tables::TableJoin::TableJoin( std::string join_type, const TableOrSubquery &new_table, const Where &join_on_clause ) {
		join = join_type;
		table = new_table.make_shared();
		on_clause = join_on_clause.make_shared();
	}


	Tables::TableJoin::TableJoin( std::string join_type, std::string new_table, SPC<Where> join_on_clause ) {
		join = join_type;
		table = std::make_shared<Table>(new_table);
		on_clause = join_on_clause;
	}


	Tables::TableJoin::TableJoin( std::string join_type, const TableOrSubquery &new_table, SPC<Where> join_on_clause ) {
		join = join_type;
		table = new_table.make_shared();
		on_clause = join_on_clause;
	}


	std::string Tables::TableJoin::to_string() const {
		if (!table)
			throw std::runtime_error("Empty TableJoin found in to_string()");

		std::string output;

		output = join + " ";
		output += table->to_string();

		if (on_clause)
			output += " ON " + on_clause->to_string();

		return output;
	}


	void Tables::TableJoin::bind( sql::PreparedStatement &pstmt, unsigned int &bind_offset ) const {
		if (!table)
			throw std::runtime_error("Empty TableJoin found in bind()");

		// call bind() on table first as it might be a subquery using its own placeholders
		table->bind(pstmt, bind_offset);

		// now we can bind any placeholders in ON clause
		if (on_clause)
			on_clause->bind(pstmt, bind_offset);
	}


	// --- Tables --- //

	Tables::Tables( std::string table ) {
		initial_table = std::make_shared<Table>(table);
	}


	Tables::Tables( const TableOrSubquery &table ) {
		initial_table = table.make_shared();
	}


	void Tables::join( std::string join_type, std::string new_table, const Where &join_on_clause ) {
		const Table t(new_table);
		join( join_type, t, join_on_clause );
	}


	void Tables::join( std::string join_type, const TableOrSubquery &new_table, const Where &join_on_clause ) {
		TableJoin table_join(join_type, new_table, join_on_clause);

		table_joins.push_back( table_join );
	}


	void Tables::join( std::string join_type, std::string new_table, SPC<Where> join_on_clause ) {
		const Table t(new_table);
		join( join_type, t, join_on_clause );
	}


	void Tables::join( std::string join_type, const TableOrSubquery &new_table, SPC<Where> join_on_clause ) {
		TableJoin table_join(join_type, new_table, join_on_clause);

		table_joins.push_back( table_join );
	}


	std::string Tables::to_string() const {
		if (!initial_table)
			return "";	// it's OK not to have ANY tables

		std::string output = " FROM " + initial_table->to_string();

		for( const auto &table_join : table_joins )
			output += " " + table_join.to_string();

		return output;
	}


	void Tables::bind( sql::PreparedStatement &pstmt, unsigned int &bind_offset ) const {
		for( const auto &table_join : table_joins )
			table_join.bind(pstmt, bind_offset);
	}

}

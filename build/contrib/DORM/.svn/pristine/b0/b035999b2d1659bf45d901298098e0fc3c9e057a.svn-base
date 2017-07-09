#ifndef DORM__INCLUDE__WHERE__HPP
#define DORM__INCLUDE__WHERE__HPP

#include "SPC.hpp"

#include <cppconn/prepared_statement.h>
#include <string>


namespace DORM {

	class ColName;
	class Default;
	class Null;


	class Where {
		public:
			virtual SPC<Where> make_shared() const =0;

			virtual std::string to_string() const =0;
			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const =0;
	};

}

#endif

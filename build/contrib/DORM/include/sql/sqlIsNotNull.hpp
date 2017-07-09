#ifndef DORM__INCLUDE__SQL__SQLISNOTNULL_HPP
#define DORM__INCLUDE__SQL__SQLISNOTNULL_HPP

#include "Where.hpp"


namespace DORM {

	class sqlIsNotNull: public Where {
		private:
			std::string		col;

		public:
			sqlIsNotNull(std::string init_col): col(init_col) {};


			virtual std::string to_string() const {
				return col + " IS NOT NULL";
			}


			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};


			virtual SPC<Where> make_shared() const { return std::make_shared<const sqlIsNotNull>(*this); };
	};

}

#endif

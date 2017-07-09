#ifndef DORM__INCLUDE__SQL__SQLISNULL_HPP
#define DORM__INCLUDE__SQL__SQLISNULL_HPP

#include "Where.hpp"


namespace DORM {

	class sqlIsNull: public Where {
		private:
			std::string		col;

		public:
			sqlIsNull(std::string init_col): col(init_col) {};


			virtual std::string to_string() const {
				return col + " IS NULL";
			}


			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};


			virtual SPC<Where> make_shared() const { return std::make_shared<const sqlIsNull>(*this); };
	};

}

#endif

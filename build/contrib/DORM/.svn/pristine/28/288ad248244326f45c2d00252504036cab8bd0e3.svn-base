#ifndef DORM__INCLUDE__SQL__SQLLIKE_HPP
#define DORM__INCLUDE__SQL__SQLLIKE_HPP

#include "Where.hpp"


namespace DORM {

	class sqlLike: public Where {
		private:
			std::string		col;
			std::string		pattern;

		public:
			sqlLike(std::string init_col, std::string init_pattern ): col(init_col), pattern(init_pattern) {};


			virtual std::string to_string() const {
				return col + " LIKE ?";
			}


			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {
				pstmt.setString(bind_offset++, pattern);
			};


			virtual SPC<Where> make_shared() const { return std::make_shared<const sqlLike>(*this); };
	};

}

#endif

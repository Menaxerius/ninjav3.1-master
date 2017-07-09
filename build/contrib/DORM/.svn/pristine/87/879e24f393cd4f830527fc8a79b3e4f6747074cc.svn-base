#ifndef DORM__INCLUDE__SQL__SQLLE_HPP
#define DORM__INCLUDE__SQL__SQLLE_HPP

#include "Where.hpp"

namespace DORM {

	template<class CXXTYPE>
	class sqlLe;

	template<>
	class sqlLe<ColName>: public Where {
		private:
			std::string		col;
			std::string		column;

		public:
			sqlLe(std::string init_col, std::string init_column): col(init_col), column(init_column) {};

			virtual std::string to_string() const {
				return col + " <= " + column;
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlLe<ColName> >(*this); };
	};


	template<>
	class sqlLe<Default>: public Where {
		private:
			std::string		col;

		public:
			sqlLe(std::string init_col): col(init_col) {};

			virtual std::string to_string() const {
				return col + " <= DEFAULT";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlLe<Default> >(*this); };
	};


	template<>
	class sqlLe<Null>: public Where {
		private:
			std::string		col;

		public:
			sqlLe(std::string init_col): col(init_col) {};

			virtual std::string to_string() const {
				return col + " <= NULL";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlLe<Null> >(*this); };
	};


	template<class CXXTYPE>
	class sqlLe: public Where {
		private:
			std::string		col;
			CXXTYPE			value;

		public:
			sqlLe(std::string init_col, CXXTYPE init_value): col(init_col), value(init_value) {};

			virtual std::string to_string() const {
				return col + " <= ?";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {
				DORM::DB::bind<CXXTYPE>(pstmt, bind_offset, value);
			};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlLe<CXXTYPE> >(*this); };
	};

}

#endif

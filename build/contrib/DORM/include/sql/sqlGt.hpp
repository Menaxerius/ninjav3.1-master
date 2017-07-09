#ifndef DORM__INCLUDE__SQL__SQLGT_HPP
#define DORM__INCLUDE__SQL__SQLGT_HPP

#include "Where.hpp"

namespace DORM {

	template<class CXXTYPE>
	class sqlGt;

	template<>
	class sqlGt<ColName>: public Where {
		private:
			std::string		col;
			std::string		column;

		public:
			sqlGt(std::string init_col, std::string init_column): col(init_col), column(init_column) {};

			virtual std::string to_string() const {
				return col + " > " + column;
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlGt<ColName> >(*this); };
	};


	template<>
	class sqlGt<Default>: public Where {
		private:
			std::string		col;

		public:
			sqlGt(std::string init_col): col(init_col) {};

			virtual std::string to_string() const {
				return col + " > DEFAULT";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlGt<Default> >(*this); };
	};


	template<>
	class sqlGt<Null>: public Where {
		private:
			std::string		col;

		public:
			sqlGt(std::string init_col): col(init_col) {};

			virtual std::string to_string() const {
				return col + " > NULL";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlGt<Null> >(*this); };
	};


	template<class CXXTYPE>
	class sqlGt: public Where {
		private:
			std::string		col;
			CXXTYPE			value;

		public:
			sqlGt(std::string init_col, CXXTYPE init_value): col(init_col), value(init_value) {};

			virtual std::string to_string() const {
				return col + " > ?";
			};

			virtual void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset) const {
				DORM::DB::bind<CXXTYPE>(pstmt, bind_offset, value);
			};

			virtual SPC<Where> make_shared() const { return std::make_shared< const sqlGt<CXXTYPE> >(*this); };
	};

}

#endif

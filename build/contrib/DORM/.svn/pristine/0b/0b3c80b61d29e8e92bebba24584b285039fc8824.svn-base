#ifndef DORM__INCLUDE__COLUMN__HPP
#define DORM__INCLUDE__COLUMN__HPP

#include "SPC.hpp"
#include "DB.hpp"
#include "sql/sqlEq.hpp"

#include <algorithm>


namespace DORM {

	class Where;


	class Column {
		private:
			class ValueInterface {
				public:
					virtual ~ValueInterface() {};

					virtual ValueInterface *clone() const =0;

					virtual SPC<Where> column_eq(const std::string &name) const =0;
			};


			template<typename CXXTYPE>
			class Value: public ValueInterface {
				public:
					CXXTYPE value;

					Value(const CXXTYPE &new_value): value(new_value) {};
					virtual ~Value() {};

					virtual ValueInterface *clone() const { return new Value(value); };

					virtual SPC<Where> column_eq(const std::string &name) const { return sqlEq<CXXTYPE>(name, value).make_shared(); };
			};


			ValueInterface *value_impl;

		public:
			bool			changed;
			bool			exists;
			bool			defined;

			Column(): value_impl(nullptr) {};

			template<typename CXXTYPE>
			Column(const CXXTYPE initial_value) { value_impl = new Value<CXXTYPE>(initial_value); };

			Column(const Column &other): value_impl( other.value_impl ? other.value_impl->clone() : nullptr ) {};
			Column(Column &&other): value_impl(other.value_impl) { other.value_impl = nullptr; };

			virtual ~Column() { if (value_impl) delete value_impl; };

			Column &swap(Column &other) { std::swap(value_impl, other.value_impl); return *this; };

			Column &operator=(const Column &other) { Column(other).swap(*this); return *this; };	// Column(other)'s destructor cleans up our previous content (post swap)

			template<typename CXXTYPE>
			Column &operator=(const CXXTYPE &other_value) { Column(other_value).swap(*this); return *this; };

			template<typename CXXTYPE>
			operator CXXTYPE() const { return static_cast< Value<CXXTYPE> *>(value_impl)->value; };

			inline void undefine() { changed = true; exists = true; defined = false; };
			inline void remove() { changed = true; exists = false; defined = false; };
			inline void define() { changed = true; exists = true; defined = true; };

			SPC<Where> column_eq(const std::string &name) const { return value_impl->column_eq(name); };

		private:
			template<typename CXXTYPE>
			friend CXXTYPE column_cast(const Column &column);

			template<typename CXXTYPE>
			Value<CXXTYPE> *impl_cast() const { return static_cast< Value<CXXTYPE> *>(value_impl); };
	};

}

#endif

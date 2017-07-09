#include "Column.hpp"


//namespace DORM {

	template<typename CXXTYPE>
	CXXTYPE column_cast(const DORM::Column &column) {
		// return static_cast< DORM::Column::Value<CXXTYPE> *>(column.value_impl)->value;
		return column.impl_cast<CXXTYPE *>()->value;
	}

//}

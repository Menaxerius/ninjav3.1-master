#include "Transaction.hpp"
#include "DB.hpp"


namespace DORM {

	Transaction::Transaction( default_action_enum action ) {
		default_action = action;
		is_active = true;

		DB::execute("START TRANSACTION");
	}


	Transaction::Transaction(Transaction &&other) noexcept {
		default_action = other.default_action;
		is_active = other.is_active;

		other.is_active = false;
	}


	Transaction::~Transaction() {
		if (!is_active)
			return;

		if (default_action == ROLLBACK)
			rollback();
		else
			commit();
	}


	void Transaction::rollback() {
		DB::execute("ROLLBACK");
		is_active = false;
	}


	void Transaction::commit() {
		DB::execute("COMMIT");
		is_active = false;
	}

}

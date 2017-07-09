#ifndef DORM__INCLUDE__TRANSACTION_HPP
#define DORM__INCLUDE__TRANSACTION_HPP

namespace DORM {

	class Transaction {
		public:
			enum default_action_enum { COMMIT, ROLLBACK };

		private:
			default_action_enum default_action;
			bool is_active;

		public:
			Transaction( default_action_enum action = ROLLBACK );
			Transaction(const Transaction &) =delete;
			Transaction(Transaction &&other) noexcept;
			virtual ~Transaction();

			void commit();
			void rollback();
	};

}

#endif

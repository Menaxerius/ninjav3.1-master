// GENERATED - DO NOT EDIT!

#ifndef DORM__Account___HXX
#define DORM__Account___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Account;

// other classes



class Account_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Account_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Accounts"; };
		virtual const int get_autoinc_index() const { return 0; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Accounts"; };

	public:
		virtual void clear();

		Account_() { columns.resize(11); clear(); };

		
			inline uint64_t accountID() const { return columns[0]; };
			inline void accountID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_accountID() { columns[0].undefine(); };
			inline void delete_accountID() { columns[0].remove(); };

			inline bool defined_accountID() { return columns[0].defined; };
			inline bool exists_accountID() { return columns[0].exists; };
		
			inline DORM::Timestamp first_seen_when() const { return columns[1]; };
			inline void first_seen_when( const DORM::Timestamp &value) { columns[1] = value; columns[1].define(); };

			inline void undef_first_seen_when() { columns[1].undefine(); };
			inline void delete_first_seen_when() { columns[1].remove(); };

			inline bool defined_first_seen_when() { return columns[1].defined; };
			inline bool exists_first_seen_when() { return columns[1].exists; };
		
			inline uint64_t reward_recipient() const { return columns[2]; };
			inline void reward_recipient( const uint64_t &value) { columns[2] = value; columns[2].define(); };

			inline void undef_reward_recipient() { columns[2].undefine(); };
			inline void delete_reward_recipient() { columns[2].remove(); };

			inline bool defined_reward_recipient() { return columns[2].defined; };
			inline bool exists_reward_recipient() { return columns[2].exists; };
		
			inline DORM::Timestamp last_updated_when() const { return columns[3]; };
			inline void last_updated_when( const DORM::Timestamp &value) { columns[3] = value; columns[3].define(); };

			inline void undef_last_updated_when() { columns[3].undefine(); };
			inline void delete_last_updated_when() { columns[3].remove(); };

			inline bool defined_last_updated_when() { return columns[3].defined; };
			inline bool exists_last_updated_when() { return columns[3].exists; };
		
			inline std::string account_name() const { return columns[4]; };
			inline void account_name( const std::string &value) { columns[4] = value; columns[4].define(); };

			inline void undef_account_name() { columns[4].undefine(); };
			inline void delete_account_name() { columns[4].remove(); };

			inline bool defined_account_name() { return columns[4].defined; };
			inline bool exists_account_name() { return columns[4].exists; };
		
			inline uint64_t mining_capacity() const { return columns[5]; };
			inline void mining_capacity( const uint64_t &value) { columns[5] = value; columns[5].define(); };

			inline void undef_mining_capacity() { columns[5].undefine(); };
			inline void delete_mining_capacity() { columns[5].remove(); };

			inline bool defined_mining_capacity() { return columns[5].defined; };
			inline bool exists_mining_capacity() { return columns[5].exists; };
		
			inline bool is_capacity_estimated() const { return columns[6]; };
			inline void is_capacity_estimated( const bool &value) { columns[6] = value; columns[6].define(); };

			inline void undef_is_capacity_estimated() { columns[6].undefine(); };
			inline void delete_is_capacity_estimated() { columns[6].remove(); };

			inline bool defined_is_capacity_estimated() { return columns[6].defined; };
			inline bool exists_is_capacity_estimated() { return columns[6].exists; };
		
			inline std::string account_RS_string() const { return columns[7]; };
			inline void account_RS_string( const std::string &value) { columns[7] = value; columns[7].define(); };

			inline void undef_account_RS_string() { columns[7].undefine(); };
			inline void delete_account_RS_string() { columns[7].remove(); };

			inline bool defined_account_RS_string() { return columns[7].defined; };
			inline bool exists_account_RS_string() { return columns[7].exists; };
		
			inline bool has_used_this_pool() const { return columns[8]; };
			inline void has_used_this_pool( const bool &value) { columns[8] = value; columns[8].define(); };

			inline void undef_has_used_this_pool() { columns[8].undefine(); };
			inline void delete_has_used_this_pool() { columns[8].remove(); };

			inline bool defined_has_used_this_pool() { return columns[8].defined; };
			inline bool exists_has_used_this_pool() { return columns[8].exists; };
		
			inline uint64_t last_checked_at_block() const { return columns[9]; };
			inline void last_checked_at_block( const uint64_t &value) { columns[9] = value; columns[9].define(); };

			inline void undef_last_checked_at_block() { columns[9].undefine(); };
			inline void delete_last_checked_at_block() { columns[9].remove(); };

			inline bool defined_last_checked_at_block() { return columns[9].defined; };
			inline bool exists_last_checked_at_block() { return columns[9].exists; };
		
			inline DORM::Timestamp last_nonce_when() const { return columns[10]; };
			inline void last_nonce_when( const DORM::Timestamp &value) { columns[10] = value; columns[10].define(); };

			inline void undef_last_nonce_when() { columns[10].undefine(); };
			inline void delete_last_nonce_when() { columns[10].remove(); };

			inline bool defined_last_nonce_when() { return columns[10].defined; };
			inline bool exists_last_nonce_when() { return columns[10].exists; };
		

		virtual std::unique_ptr<Account> load();
		static std::unique_ptr<Account> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Account> load( uint64_t key_accountID );

		virtual std::unique_ptr<Account> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Account> clone() const;

		// navigators
		
};

#endif

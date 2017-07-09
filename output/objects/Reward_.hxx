// GENERATED - DO NOT EDIT!

#ifndef DORM__Reward___HXX
#define DORM__Reward___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Reward;

// other classes

	class Account;

	class Block;

	class Bonus;



class Reward_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Reward_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Rewards"; };
		virtual const int get_autoinc_index() const { return 1; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Rewards"; };

	public:
		virtual void clear();

		Reward_() { columns.resize(9); clear(); };

		
			inline uint64_t rewardID() const { return columns[0]; };
			inline void rewardID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_rewardID() { columns[0].undefine(); };
			inline void delete_rewardID() { columns[0].remove(); };

			inline bool defined_rewardID() { return columns[0].defined; };
			inline bool exists_rewardID() { return columns[0].exists; };
		
			inline uint64_t accountID() const { return columns[1]; };
			inline void accountID( const uint64_t &value) { columns[1] = value; columns[1].define(); };

			inline void undef_accountID() { columns[1].undefine(); };
			inline void delete_accountID() { columns[1].remove(); };

			inline bool defined_accountID() { return columns[1].defined; };
			inline bool exists_accountID() { return columns[1].exists; };
		
			inline uint64_t blockID() const { return columns[2]; };
			inline void blockID( const uint64_t &value) { columns[2] = value; columns[2].define(); };

			inline void undef_blockID() { columns[2].undefine(); };
			inline void delete_blockID() { columns[2].remove(); };

			inline bool defined_blockID() { return columns[2].defined; };
			inline bool exists_blockID() { return columns[2].exists; };
		
			inline uint64_t bonusID() const { return columns[3]; };
			inline void bonusID( const uint64_t &value) { columns[3] = value; columns[3].define(); };

			inline void undef_bonusID() { columns[3].undefine(); };
			inline void delete_bonusID() { columns[3].remove(); };

			inline bool defined_bonusID() { return columns[3].defined; };
			inline bool exists_bonusID() { return columns[3].exists; };
		
			inline uint64_t amount() const { return columns[4]; };
			inline void amount( const uint64_t &value) { columns[4] = value; columns[4].define(); };

			inline void undef_amount() { columns[4].undefine(); };
			inline void delete_amount() { columns[4].remove(); };

			inline bool defined_amount() { return columns[4].defined; };
			inline bool exists_amount() { return columns[4].exists; };
		
			inline bool is_paid() const { return columns[5]; };
			inline void is_paid( const bool &value) { columns[5] = value; columns[5].define(); };

			inline void undef_is_paid() { columns[5].undefine(); };
			inline void delete_is_paid() { columns[5].remove(); };

			inline bool defined_is_paid() { return columns[5].defined; };
			inline bool exists_is_paid() { return columns[5].exists; };
		
			inline uint64_t tx_id() const { return columns[6]; };
			inline void tx_id( const uint64_t &value) { columns[6] = value; columns[6].define(); };

			inline void undef_tx_id() { columns[6].undefine(); };
			inline void delete_tx_id() { columns[6].remove(); };

			inline bool defined_tx_id() { return columns[6].defined; };
			inline bool exists_tx_id() { return columns[6].exists; };
		
			inline bool is_confirmed() const { return columns[7]; };
			inline void is_confirmed( const bool &value) { columns[7] = value; columns[7].define(); };

			inline void undef_is_confirmed() { columns[7].undefine(); };
			inline void delete_is_confirmed() { columns[7].remove(); };

			inline bool defined_is_confirmed() { return columns[7].defined; };
			inline bool exists_is_confirmed() { return columns[7].exists; };
		
			inline uint64_t paid_at_block_id() const { return columns[8]; };
			inline void paid_at_block_id( const uint64_t &value) { columns[8] = value; columns[8].define(); };

			inline void undef_paid_at_block_id() { columns[8].undefine(); };
			inline void delete_paid_at_block_id() { columns[8].remove(); };

			inline bool defined_paid_at_block_id() { return columns[8].defined; };
			inline bool exists_paid_at_block_id() { return columns[8].exists; };
		

		virtual std::unique_ptr<Reward> load();
		static std::unique_ptr<Reward> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Reward> load( uint64_t key_rewardID );

		virtual std::unique_ptr<Reward> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Reward> clone() const;

		// navigators
		
			virtual std::unique_ptr<Account> account() const;
		
			virtual std::unique_ptr<Block> block() const;
		
			virtual std::unique_ptr<Bonus> bonus() const;
		
};

#endif

// GENERATED - DO NOT EDIT!

#ifndef DORM__Block___HXX
#define DORM__Block___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Block;

// other classes

	class Nonce;

	class Share;

	class Reward;



class Block_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Block_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Blocks"; };
		virtual const int get_autoinc_index() const { return 0; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Blocks"; };

	public:
		virtual void clear();

		Block_() { columns.resize(16); clear(); };

		
			inline uint64_t blockID() const { return columns[0]; };
			inline void blockID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_blockID() { columns[0].undefine(); };
			inline void delete_blockID() { columns[0].remove(); };

			inline bool defined_blockID() { return columns[0].defined; };
			inline bool exists_blockID() { return columns[0].exists; };
		
			inline DORM::Timestamp first_seen_when() const { return columns[1]; };
			inline void first_seen_when( const DORM::Timestamp &value) { columns[1] = value; columns[1].define(); };

			inline void undef_first_seen_when() { columns[1].undefine(); };
			inline void delete_first_seen_when() { columns[1].remove(); };

			inline bool defined_first_seen_when() { return columns[1].defined; };
			inline bool exists_first_seen_when() { return columns[1].exists; };
		
			inline uint64_t best_nonce_account_id() const { return columns[2]; };
			inline void best_nonce_account_id( const uint64_t &value) { columns[2] = value; columns[2].define(); };

			inline void undef_best_nonce_account_id() { columns[2].undefine(); };
			inline void delete_best_nonce_account_id() { columns[2].remove(); };

			inline bool defined_best_nonce_account_id() { return columns[2].defined; };
			inline bool exists_best_nonce_account_id() { return columns[2].exists; };
		
			inline uint64_t generator_account_id() const { return columns[3]; };
			inline void generator_account_id( const uint64_t &value) { columns[3] = value; columns[3].define(); };

			inline void undef_generator_account_id() { columns[3].undefine(); };
			inline void delete_generator_account_id() { columns[3].remove(); };

			inline bool defined_generator_account_id() { return columns[3].defined; };
			inline bool exists_generator_account_id() { return columns[3].exists; };
		
			inline uint64_t block_reward() const { return columns[4]; };
			inline void block_reward( const uint64_t &value) { columns[4] = value; columns[4].define(); };

			inline void undef_block_reward() { columns[4].undefine(); };
			inline void delete_block_reward() { columns[4].remove(); };

			inline bool defined_block_reward() { return columns[4].defined; };
			inline bool exists_block_reward() { return columns[4].exists; };
		
			inline bool is_our_block() const { return columns[5]; };
			inline void is_our_block( const bool &value) { columns[5] = value; columns[5].define(); };

			inline void undef_is_our_block() { columns[5].undefine(); };
			inline void delete_is_our_block() { columns[5].remove(); };

			inline bool defined_is_our_block() { return columns[5].defined; };
			inline bool exists_is_our_block() { return columns[5].exists; };
		
			inline bool has_been_shared() const { return columns[6]; };
			inline void has_been_shared( const bool &value) { columns[6] = value; columns[6].define(); };

			inline void undef_has_been_shared() { columns[6].undefine(); };
			inline void delete_has_been_shared() { columns[6].remove(); };

			inline bool defined_has_been_shared() { return columns[6].defined; };
			inline bool exists_has_been_shared() { return columns[6].exists; };
		
			inline uint64_t base_target() const { return columns[7]; };
			inline void base_target( const uint64_t &value) { columns[7] = value; columns[7].define(); };

			inline void undef_base_target() { columns[7].undefine(); };
			inline void delete_base_target() { columns[7].remove(); };

			inline bool defined_base_target() { return columns[7].defined; };
			inline bool exists_base_target() { return columns[7].exists; };
		
			inline DORM::Timestamp forged_when() const { return columns[8]; };
			inline void forged_when( const DORM::Timestamp &value) { columns[8] = value; columns[8].define(); };

			inline void undef_forged_when() { columns[8].undefine(); };
			inline void delete_forged_when() { columns[8].remove(); };

			inline bool defined_forged_when() { return columns[8].defined; };
			inline bool exists_forged_when() { return columns[8].exists; };
		
			inline uint32_t scoop() const { return columns[9]; };
			inline void scoop( const uint32_t &value) { columns[9] = value; columns[9].define(); };

			inline void undef_scoop() { columns[9].undefine(); };
			inline void delete_scoop() { columns[9].remove(); };

			inline bool defined_scoop() { return columns[9].defined; };
			inline bool exists_scoop() { return columns[9].exists; };
		
			inline uint64_t nonce() const { return columns[10]; };
			inline void nonce( const uint64_t &value) { columns[10] = value; columns[10].define(); };

			inline void undef_nonce() { columns[10].undefine(); };
			inline void delete_nonce() { columns[10].remove(); };

			inline bool defined_nonce() { return columns[10].defined; };
			inline bool exists_nonce() { return columns[10].exists; };
		
			inline std::string generation_signature() const { return columns[11]; };
			inline void generation_signature( const std::string &value) { columns[11] = value; columns[11].define(); };

			inline void undef_generation_signature() { columns[11].undefine(); };
			inline void delete_generation_signature() { columns[11].remove(); };

			inline bool defined_generation_signature() { return columns[11].defined; };
			inline bool exists_generation_signature() { return columns[11].exists; };
		
			inline uint32_t deadline() const { return columns[12]; };
			inline void deadline( const uint32_t &value) { columns[12] = value; columns[12].define(); };

			inline void undef_deadline() { columns[12].undefine(); };
			inline void delete_deadline() { columns[12].remove(); };

			inline bool defined_deadline() { return columns[12].defined; };
			inline bool exists_deadline() { return columns[12].exists; };
		
			inline uint32_t our_best_deadline() const { return columns[13]; };
			inline void our_best_deadline( const uint32_t &value) { columns[13] = value; columns[13].define(); };

			inline void undef_our_best_deadline() { columns[13].undefine(); };
			inline void delete_our_best_deadline() { columns[13].remove(); };

			inline bool defined_our_best_deadline() { return columns[13].defined; };
			inline bool exists_our_best_deadline() { return columns[13].exists; };
		
			inline uint32_t num_potential_miners() const { return columns[14]; };
			inline void num_potential_miners( const uint32_t &value) { columns[14] = value; columns[14].define(); };

			inline void undef_num_potential_miners() { columns[14].undefine(); };
			inline void delete_num_potential_miners() { columns[14].remove(); };

			inline bool defined_num_potential_miners() { return columns[14].defined; };
			inline bool exists_num_potential_miners() { return columns[14].exists; };
		
			inline uint32_t num_rejected_nonces() const { return columns[15]; };
			inline void num_rejected_nonces( const uint32_t &value) { columns[15] = value; columns[15].define(); };

			inline void undef_num_rejected_nonces() { columns[15].undefine(); };
			inline void delete_num_rejected_nonces() { columns[15].remove(); };

			inline bool defined_num_rejected_nonces() { return columns[15].defined; };
			inline bool exists_num_rejected_nonces() { return columns[15].exists; };
		

		virtual std::unique_ptr<Block> load();
		static std::unique_ptr<Block> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Block> load( uint64_t key_blockID );

		virtual std::unique_ptr<Block> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Block> clone() const;

		// navigators
		
			virtual std::unique_ptr<Nonce> block_nonces() const;
		
			virtual std::unique_ptr<Share> block_shares() const;
		
			virtual std::unique_ptr<Reward> block_rewards() const;
		
};

#endif

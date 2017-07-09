// GENERATED - DO NOT EDIT!

#ifndef DORM__Nonce___HXX
#define DORM__Nonce___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Nonce;

// other classes

	class Account;

	class Block;



class Nonce_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Nonce_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Nonces"; };
		virtual const int get_autoinc_index() const { return 0; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Nonces"; };

	public:
		virtual void clear();

		Nonce_() { columns.resize(8); clear(); };

		
			inline uint64_t accountID() const { return columns[0]; };
			inline void accountID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_accountID() { columns[0].undefine(); };
			inline void delete_accountID() { columns[0].remove(); };

			inline bool defined_accountID() { return columns[0].defined; };
			inline bool exists_accountID() { return columns[0].exists; };
		
			inline uint64_t blockID() const { return columns[1]; };
			inline void blockID( const uint64_t &value) { columns[1] = value; columns[1].define(); };

			inline void undef_blockID() { columns[1].undefine(); };
			inline void delete_blockID() { columns[1].remove(); };

			inline bool defined_blockID() { return columns[1].defined; };
			inline bool exists_blockID() { return columns[1].exists; };
		
			inline uint64_t nonce() const { return columns[2]; };
			inline void nonce( const uint64_t &value) { columns[2] = value; columns[2].define(); };

			inline void undef_nonce() { columns[2].undefine(); };
			inline void delete_nonce() { columns[2].remove(); };

			inline bool defined_nonce() { return columns[2].defined; };
			inline bool exists_nonce() { return columns[2].exists; };
		
			inline DORM::Timestamp submitted_when() const { return columns[3]; };
			inline void submitted_when( const DORM::Timestamp &value) { columns[3] = value; columns[3].define(); };

			inline void undef_submitted_when() { columns[3].undefine(); };
			inline void delete_submitted_when() { columns[3].remove(); };

			inline bool defined_submitted_when() { return columns[3].defined; };
			inline bool exists_submitted_when() { return columns[3].exists; };
		
			inline uint64_t deadline() const { return columns[4]; };
			inline void deadline( const uint64_t &value) { columns[4] = value; columns[4].define(); };

			inline void undef_deadline() { columns[4].undefine(); };
			inline void delete_deadline() { columns[4].remove(); };

			inline bool defined_deadline() { return columns[4].defined; };
			inline bool exists_deadline() { return columns[4].exists; };
		
			inline std::string deadline_string() const { return columns[5]; };
			inline void deadline_string( const std::string &value) { columns[5] = value; columns[5].define(); };

			inline void undef_deadline_string() { columns[5].undefine(); };
			inline void delete_deadline_string() { columns[5].remove(); };

			inline bool defined_deadline_string() { return columns[5].defined; };
			inline bool exists_deadline_string() { return columns[5].exists; };
		
			inline DORM::Timestamp forge_when() const { return columns[6]; };
			inline void forge_when( const DORM::Timestamp &value) { columns[6] = value; columns[6].define(); };

			inline void undef_forge_when() { columns[6].undefine(); };
			inline void delete_forge_when() { columns[6].remove(); };

			inline bool defined_forge_when() { return columns[6].defined; };
			inline bool exists_forge_when() { return columns[6].exists; };
		
			inline std::string miner() const { return columns[7]; };
			inline void miner( const std::string &value) { columns[7] = value; columns[7].define(); };

			inline void undef_miner() { columns[7].undefine(); };
			inline void delete_miner() { columns[7].remove(); };

			inline bool defined_miner() { return columns[7].defined; };
			inline bool exists_miner() { return columns[7].exists; };
		

		virtual std::unique_ptr<Nonce> load();
		static std::unique_ptr<Nonce> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Nonce> load( uint64_t key_accountID, uint64_t key_blockID, uint64_t key_nonce );

		virtual std::unique_ptr<Nonce> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Nonce> clone() const;

		// navigators
		
			virtual std::unique_ptr<Account> account() const;
		
			virtual std::unique_ptr<Block> block() const;
		
};

#endif

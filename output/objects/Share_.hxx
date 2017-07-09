// GENERATED - DO NOT EDIT!

#ifndef DORM__Share___HXX
#define DORM__Share___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Share;

// other classes

	class Block;

	class Account;



class Share_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Share_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Shares"; };
		virtual const int get_autoinc_index() const { return 0; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Shares"; };

	public:
		virtual void clear();

		Share_() { columns.resize(6); clear(); };

		
			inline uint64_t blockID() const { return columns[0]; };
			inline void blockID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_blockID() { columns[0].undefine(); };
			inline void delete_blockID() { columns[0].remove(); };

			inline bool defined_blockID() { return columns[0].defined; };
			inline bool exists_blockID() { return columns[0].exists; };
		
			inline uint64_t accountID() const { return columns[1]; };
			inline void accountID( const uint64_t &value) { columns[1] = value; columns[1].define(); };

			inline void undef_accountID() { columns[1].undefine(); };
			inline void delete_accountID() { columns[1].remove(); };

			inline bool defined_accountID() { return columns[1].defined; };
			inline bool exists_accountID() { return columns[1].exists; };
		
			inline double share_fraction() const { return columns[2]; };
			inline void share_fraction( const double &value) { columns[2] = value; columns[2].define(); };

			inline void undef_share_fraction() { columns[2].undefine(); };
			inline void delete_share_fraction() { columns[2].remove(); };

			inline bool defined_share_fraction() { return columns[2].defined; };
			inline bool exists_share_fraction() { return columns[2].exists; };
		
			inline uint32_t deadline() const { return columns[3]; };
			inline void deadline( const uint32_t &value) { columns[3] = value; columns[3].define(); };

			inline void undef_deadline() { columns[3].undefine(); };
			inline void delete_deadline() { columns[3].remove(); };

			inline bool defined_deadline() { return columns[3].defined; };
			inline bool exists_deadline() { return columns[3].exists; };
		
			inline std::string deadline_string() const { return columns[4]; };
			inline void deadline_string( const std::string &value) { columns[4] = value; columns[4].define(); };

			inline void undef_deadline_string() { columns[4].undefine(); };
			inline void delete_deadline_string() { columns[4].remove(); };

			inline bool defined_deadline_string() { return columns[4].defined; };
			inline bool exists_deadline_string() { return columns[4].exists; };
		
			inline std::string miner() const { return columns[5]; };
			inline void miner( const std::string &value) { columns[5] = value; columns[5].define(); };

			inline void undef_miner() { columns[5].undefine(); };
			inline void delete_miner() { columns[5].remove(); };

			inline bool defined_miner() { return columns[5].defined; };
			inline bool exists_miner() { return columns[5].exists; };
		

		virtual std::unique_ptr<Share> load();
		static std::unique_ptr<Share> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Share> load( uint64_t key_blockID, uint64_t key_accountID );

		virtual std::unique_ptr<Share> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Share> clone() const;

		// navigators
		
			virtual std::unique_ptr<Block> block() const;
		
			virtual std::unique_ptr<Account> account() const;
		
};

#endif

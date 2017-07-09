// GENERATED - DO NOT EDIT!

#ifndef DORM__Bonus___HXX
#define DORM__Bonus___HXX

// empty defines
#define CHILD_OBJECT(obj, nav)
#define CHILD_OBJECTS(obj, nav)

#include "Object.hpp"
#include "Timestamp.hpp"

// for searchmod support
#include "SearchMod.hpp"
#include "Query.hpp"


// our class
class Bonus;

// other classes



class Bonus_: public DORM::Object {
	private:
		static const std::vector<Info> column_info;
		static const std::vector< std::function< void(const DORM::Resultset &result, Bonus_ &obj) > > column_resultset_function;

		virtual const std::vector<Info> &get_column_info() const { return column_info; };
		virtual const std::string get_table_name() const { return "Bonuses"; };
		virtual const int get_autoinc_index() const { return 1; };

		std::unique_ptr<Object> make_unique();

		void column_from_resultset( int i, const DORM::Resultset &result );

	protected:
		static const std::string static_table_name() { return "Bonuses"; };

	public:
		virtual void clear();

		Bonus_() { columns.resize(4); clear(); };

		
			inline uint64_t bonusID() const { return columns[0]; };
			inline void bonusID( const uint64_t &value) { columns[0] = value; columns[0].define(); };

			inline void undef_bonusID() { columns[0].undefine(); };
			inline void delete_bonusID() { columns[0].remove(); };

			inline bool defined_bonusID() { return columns[0].defined; };
			inline bool exists_bonusID() { return columns[0].exists; };
		
			inline uint64_t tx_id() const { return columns[1]; };
			inline void tx_id( const uint64_t &value) { columns[1] = value; columns[1].define(); };

			inline void undef_tx_id() { columns[1].undefine(); };
			inline void delete_tx_id() { columns[1].remove(); };

			inline bool defined_tx_id() { return columns[1].defined; };
			inline bool exists_tx_id() { return columns[1].exists; };
		
			inline uint64_t amount() const { return columns[2]; };
			inline void amount( const uint64_t &value) { columns[2] = value; columns[2].define(); };

			inline void undef_amount() { columns[2].undefine(); };
			inline void delete_amount() { columns[2].remove(); };

			inline bool defined_amount() { return columns[2].defined; };
			inline bool exists_amount() { return columns[2].exists; };
		
			inline DORM::Timestamp seen_when() const { return columns[3]; };
			inline void seen_when( const DORM::Timestamp &value) { columns[3] = value; columns[3].define(); };

			inline void undef_seen_when() { columns[3].undefine(); };
			inline void delete_seen_when() { columns[3].remove(); };

			inline bool defined_seen_when() { return columns[3].defined; };
			inline bool exists_seen_when() { return columns[3].exists; };
		

		virtual std::unique_ptr<Bonus> load();
		static std::unique_ptr<Bonus> load(const Object &obj);
		// load using keys
		static std::unique_ptr<Bonus> load( uint64_t key_bonusID );

		virtual std::unique_ptr<Bonus> result();
		virtual void search_and_destroy();

		virtual std::unique_ptr<Bonus> clone() const;

		// navigators
		
};

#endif

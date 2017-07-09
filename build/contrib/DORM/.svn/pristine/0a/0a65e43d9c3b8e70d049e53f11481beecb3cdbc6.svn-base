#ifndef DORM__INCLUDE__LOCK_HPP
#define DORM__INCLUDE__LOCK_HPP

/*
 * WARNING: Holding more than one lock per connection is only supported in mySQL v5.7.5 or later
 */



#include <string>
#include <stdexcept>


namespace DORM {

	class Lock {
		private:
			std::string name;
			bool is_active;

		public:
			class Exception: public std::runtime_error {
				public:
					Exception( const std::string &reason ): std::runtime_error(reason) {};
			};


			Lock( std::string lock_name, int timeout = -1 );
			Lock(const Lock &) =delete;		// doesn't make sense to copy a lock
			Lock(Lock &&) =default;			// can transfer lock ownership though
			virtual ~Lock();

			void release();
	};

}

#endif

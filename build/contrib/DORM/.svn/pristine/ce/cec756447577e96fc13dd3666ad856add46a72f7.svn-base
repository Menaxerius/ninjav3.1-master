#ifndef DORM__INCLUDE__DB__HPP
#define DORM__INCLUDE__DB__HPP

#include "SPC.hpp"

#include <cppconn/connection.h>
#include <vector>
#include <string>
#include <sys/time.h>


namespace DORM {

	class Resultset;
	class Query;
	class Where;

	class DB {
		private:
			static thread_local std::unique_ptr<sql::Connection> conn;
			static std::string uri, user, password, schema;

			static void connect();

		public:
			class connection_issue : public sql::SQLException {
				public: connection_issue(const sql::SQLException &e) : sql::SQLException(e) {};
			};

			class ThreadGuard {
				private:
					static bool first_init;

				public:
					ThreadGuard();
					~ThreadGuard();
			};
			static thread_local ThreadGuard thread_guard;

			static void connect( const std::string db_uri, const std::string db_user, const std::string db_password, const std::string db_schema );
			static void connect( sql::Connection *new_conn );
			static void disconnect();

			static bool check_connection();

			static std::unique_ptr<sql::PreparedStatement> prepare( const std::string &sql );

			static Resultset *select( const Query &query );

			static uint64_t fetch_uint64( const Query &query );
			static int64_t fetch_int64( const Query &query );
			static double fetch_double( const Query &query );
			static std::string fetch_string( const Query &query );
			static bool fetch_bool( const Query &query );

			static int writerow( const std::string &table, const std::vector< SPC<Where> > &inserts_and_updates );
			static int writerow( const std::string &table, const std::vector< SPC<Where> > &inserts, const std::vector< SPC<Where> > &updates );

			static int deleterow( const std::string &table, const Where &where_clause );
			static int execute( const std::string &sql );

			static std::string from_unixtime( const time_t t );
			static std::string from_unixtime( const struct timeval &tv );
			static time_t unix_timestamp( const std::string &ts );
			static struct timeval unix_timeval( const std::string &ts );

			template<typename CXXTYPE>
			static void bind(sql::PreparedStatement &pstmt, unsigned int &bind_offset, CXXTYPE value);
	};

}

#endif

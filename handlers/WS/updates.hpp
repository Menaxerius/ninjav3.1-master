#ifndef HANDLERS__WS__UPDATES_HPP
#define HANDLERS__WS__UPDATES_HPP

#include "Nonce.hpp"

#include "WebSocketHandler.hpp"
#include "JSON.hpp"

#include <mutex>


namespace Handlers {
	namespace WS {

		class updates: public WebSocketHandler<updates> {
			private:
				static std::string					block_msg;
				static std::string					shares_msg;
				static bool							shares_msg_changed;
				static std::string					awards_msg;
				static std::string					historic_shares_msg;

				static uint64_t						latest_blockID;
				static time_t						latest_block_when;
				static std::map<uint64_t,time_t>	all_seen_accounts;

				static std::mutex					updates_mutex;

				const static time_t					started_when;

				std::map<uint64_t,time_t>			sent_accounts;

				uint64_t							our_blockID = 0;

				static void check_block();
				static void update_historic_shares();
				static void check_shares();
				static void generate_awards();
				static void add_account_award( JSON &awards, const std::string &award_name, Nonce &nonce, time_t block_start_time );
				static void update_account_info( uint64_t accountID );
				void send_new_accounts();

			public:
				static const std::string			websocket_protocol;

				updates() =default;
				updates(MHD_socket sock, struct MHD_UpgradeResponseHandle *urh, Request *req);

				static void websocket_alert_prepare();
				virtual void websocket_alert();
		};

	} // WS namespace
} // Handlers namespace


#endif

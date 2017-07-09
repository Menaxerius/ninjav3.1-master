#include "push_notification.hpp"
#include "fetch.hpp"


void push_notification( const std::string url, const std::string key, const std::string user, const std::string message, const std::string sound ) {
	std::string post_data = "token=" + key + "&user=" + user + "&message=" + message;
	if ( !sound.empty() )
		post_data += "&sound=" + sound;

	async_fetch(url, post_data);
}

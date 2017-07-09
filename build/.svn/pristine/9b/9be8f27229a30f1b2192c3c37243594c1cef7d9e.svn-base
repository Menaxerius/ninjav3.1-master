#ifndef INCLUDE__WEBSOCKETHANDLER_HPP
#define INCLUDE__WEBSOCKETHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Handler.hpp"
#include "WebSocketFrame.hpp"
#include "ftime.hpp"
#include "Base64.hpp"

#include "DORM/DB.hpp"

#include <microhttpd.h>

#include <map>
#include <thread>
#include <mutex>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

#include "pthread_np_shim.hpp"


class WebSocketHandlerCommon: public Handler {
	protected:
		MHD_socket							sock;
		struct MHD_UpgradeResponseHandle	*urh;
};


template<class SUBCLASS>
class WebSocketHandler: public WebSocketHandlerCommon {
	private:
		static const int WEBSOCKET_BUFFER_SIZE = 65536;

	protected:
		static std::map<MHD_socket, SUBCLASS *>		connections;
		static std::mutex							connections_mutex;

	private:
		static void add_connection(MHD_socket new_sock, SUBCLASS *obj);
		static void remove_connection(MHD_socket old_sock);
		static void make_blocking(MHD_socket fd);
		// generic receive thread
		virtual void websocket_receive_loop(SUBCLASS *obj, MHD_socket sock, struct MHD_UpgradeResponseHandle *urh);

	protected:
		// individual connection processing data received on websocket
		virtual void websocket_receive(const WebSocketFrame &wsf) {};
		// one-off prepare before calling websocket_alert() on each connection
		static void websocket_alert_prepare() {};
		// individual connection sending update out via websocket
		virtual void websocket_alert() {};
		virtual void close_websocket();
		virtual void send_websocket_frame(const WebSocketFrame &wsf);
		static void async_wake_up();

	public:
		// ask all connections if they want to send updates
		static void wake_up();
		static void upgrade_handler(void *cls, struct MHD_Connection *connection, void *con_cls, const char *extra_in, size_t extra_in_size, MHD_socket sock, struct MHD_UpgradeResponseHandle *urh);
		virtual int process( struct MHD_Connection *connection, Request *req, Response *resp );
};


template<class SUBCLASS>
std::map<MHD_socket, SUBCLASS *> WebSocketHandler<SUBCLASS>::connections;

template<class SUBCLASS>
std::mutex WebSocketHandler<SUBCLASS>::connections_mutex;


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::add_connection(MHD_socket new_sock, SUBCLASS *obj) {
	std::lock_guard<std::mutex> mutex_lock(connections_mutex);

	connections[new_sock] = obj;
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::remove_connection(MHD_socket old_sock) {
	std::lock_guard<std::mutex> mutex_lock(connections_mutex);

	const auto &it = connections.find(old_sock);

	// grab pointer to object so we can call destroy it after it's removed from connections map
	SUBCLASS *obj = it->second;

	// remove from connections map
	connections.erase(it);

	// actually delete connection
	delete obj;
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::make_blocking(MHD_socket fd) {
	using namespace std::literals::string_literals;

	int flags = fcntl(fd, F_GETFL);

	if (flags == -1)
		throw std::runtime_error( "Can't get socket flags: "s + std::strerror(errno) );

	if ( (flags & ~O_NONBLOCK) != flags ) {
		const int ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

		if (ret == -1)
			throw std::runtime_error( "Can't set socket to blocking mode: "s + std::strerror(errno) );
	}
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::websocket_receive_loop(SUBCLASS *obj, MHD_socket sock, struct MHD_UpgradeResponseHandle *urh) {
    pthread_set_name_np(pthread_self(), "websocket-recv");

	make_blocking(sock);

	add_connection(sock, obj);

	// opportunity to send initial output
	websocket_alert();

	// raw socket buffer
	char buffer[WEBSOCKET_BUFFER_SIZE];
	size_t buffer_offset = 0;

	while(1) {
		const int nread = read( sock, buffer + buffer_offset, sizeof(buffer) - buffer_offset );

		if (nread == -1)
			break;		// something bad - give up

		if (nread == 0)
			break;		// EOF - socket closed

		buffer_offset += nread;

		// something read?
		WebSocketFrame wsf;

		size_t payload_offset = 0;
		do {
			size_t next_offset = wsf.unpack(buffer + payload_offset, buffer_offset - payload_offset);

			if (next_offset == 0)
				break;	// failed to decode payload - wait for more input

			if (wsf.opcode == WebSocketFrame::WS_CLOSE)
				break;

			websocket_receive(wsf);

			payload_offset += next_offset;
		} while( payload_offset < buffer_offset + nread );

		if (wsf.opcode == WebSocketFrame::WS_CLOSE)
			break;

		// tidy up buffer
		memmove(buffer, buffer + payload_offset, buffer_offset - payload_offset);
		buffer_offset -= payload_offset;
	}

	// remove connection from "connections"
	remove_connection(sock);

	MHD_upgrade_action(urh, MHD_UPGRADE_ACTION_CLOSE);
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::close_websocket() {
	// calling shutdown causes read() in receive thread to return
	shutdown(sock, SHUT_RDWR);
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::send_websocket_frame(const WebSocketFrame &wsf) {
	// don't send empty frames
	if (wsf.payload_len == 0)
		return;

	int nwrote = write(sock, wsf.payload_data, wsf.payload_len);

	if (nwrote == -1) {
		std::cerr << "Websocket frame write() failed: " << std::strerror(errno) << std::endl;
		close_websocket();
	}
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::async_wake_up() {

	try {
		DORM::DB::check_connection();

		// wait on a mutex to serialize wake_up()s / prevent races
		// also stops corruption to connections iterator
		// we only need one thread doing wake_up
		if ( !connections_mutex.try_lock() )
			return;

		// prepare before calling websocket_alert for each connection
		// e.g. read data that is common to all alerts
		SUBCLASS::websocket_alert_prepare();

		std::vector<std::thread> alert_threads;

		for(const auto &conn_pair : connections) {
			std::thread alert_thread = std::thread( &SUBCLASS::websocket_alert, conn_pair.second );
			alert_threads.push_back( std::move(alert_thread) );
		}

		for(auto &alert_thread : alert_threads)
			alert_thread.join();

		connections_mutex.unlock();
	} catch(const DORM::DB::connection_issue &e) {
		// non-fatal
	} catch(const std::exception &e) {
		// we still need to release mutex
		throw(e);
	}
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::wake_up() {
	// let caller continue ASAP
	std::thread( &SUBCLASS::async_wake_up ).detach();
}


template<class SUBCLASS>
void WebSocketHandler<SUBCLASS>::upgrade_handler(void *cls, struct MHD_Connection *connection, void *con_cls, const char *extra_in, size_t extra_in_size, MHD_socket sock, struct MHD_UpgradeResponseHandle *urh) {
	Request *req = (Request *)(cls);

	// create specific upgrade handler
	// (adding to connections vector done async later as we're not allowed to block according to libmicrohttpd docs)
	auto upgraded_handler = new SUBCLASS(sock, urh, req);

	// spawn receive thread
	std::thread receive_thread( &SUBCLASS::websocket_receive_loop, upgraded_handler, upgraded_handler, sock, urh );
	receive_thread.detach();
}


template<class SUBCLASS>
int WebSocketHandler<SUBCLASS>::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return resp->upgrade_websocket( req, SUBCLASS::websocket_protocol, WebSocketHandler::upgrade_handler );
}


#endif

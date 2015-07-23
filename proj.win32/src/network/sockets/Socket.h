#ifndef __SOCKET_H__
#define __SOCKET_H__

/*
simple cross-platform berkeley socket class used to encapsulate simpler functions
*/

#include <base/CCConsole.h>
#include "debug/PlatformConfig.h"

#if defined(PLATFORM_WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_LINUX)
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define THROW_IF_ERROR 1

enum SocketProtocol {
	PROTO_TCP, 
	PROTO_UDP
};

class Socket {

	public:
		static void init_sockets();
		static int poll_fds(pollfd* fd_array, int array_len, int timeout);

		Socket(SocketProtocol c_protocol = PROTO_TCP, char* c_ip = "undefined", short c_port = -1);
		SocketProtocol protocol;

		int s_create();
		int s_bind();
		int s_connect();
		int s_send(char* buffer, int buffer_len);
		int s_recv(char* buffer, int buffer_len);
		int s_select(fd_set* read_set, fd_set* write_set, bool use_timeout = false, int timeout_seconds = 0, int timeout_ms = 0);
        int s_change_addr(char* c_ip, short c_port);
        int s_update_addr_info();

		uintptr_t get_sock() { return sock; }
		char* get_ip() { return ip; }
		short get_port() { return port; }
        addrinfo& get_sock_info() { return sock_info; }
        sockaddr_in& get_addr_info() { return addr_info; }

	private:
		int result;
		char* ip;
		short port;
		uintptr_t sock;
		struct sockaddr_in addr_info;
		struct addrinfo sock_info;
		fd_set* r_set = NULL;
		fd_set* w_set = NULL;
        timeval t;
        
        int throw_error(int err, char* func_err);
};

#endif

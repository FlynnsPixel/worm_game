#include "MessageRecv.h"

Socket messagerecv::tcp_sock;
Socket messagerecv::udp_sock;

void messagerecv::start() {
	Socket::init_sockets();

	tcp_sock = Socket(PROTO_TCP, "192.168.0.5", "4222");
	tcp_sock.s_create();
	tcp_sock.s_connect();
	
	udp_sock = Socket(PROTO_UDP, "192.168.0.2", "4222");
	udp_sock.s_create();
	udp_sock.s_bind();

	message::ByteStream() << message::MID_CLIENT_USER_PASS << false << true;
	CMID mid = message::extract_mid(message::byte_buffer, message::byte_offset);
	message::extract_params(mid, message::byte_buffer, message::byte_offset);

	message::send(&tcp_sock, message::ByteStream() << message::MID_CLIENT_USER_PASS << false << true);

	char buffer[1024];
	int msg_len;
		while (true) {
		if ((msg_len = tcp_sock.s_recv(buffer, sizeof(buffer))) > 0) {
			CCLOG("buffer: %s", buffer);
		}
	}
}
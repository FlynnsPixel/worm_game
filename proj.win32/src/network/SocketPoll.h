#ifndef _SOCKET_POLL_
#define _SOCKET_POLL_

/*
class to add/remove sockets and determine when each one is ready to be read (has an incoming message)
*/

#include <vector>
#include "Socket.h"

class SocketPoll {
    
    public:
        /* poll is a blocking operation */
        int poll();
        void add_sock(Socket& sock);
        void remove_sock(Socket& sock);
        void remove_sock(int at);

        int get_size() { return fds.size(); }
        pollfd* get_fd_at(int i) { return (i >= 0 && i < fds.size()) ? fds[i] : NULL; }
        Socket* get_sock_at(int i) { return (i >= 0 && i < sockets.size()) ? sockets[i] : NULL; }

    private:
        std::vector<pollfd*> fds;
        std::vector<Socket*> sockets;
};

#endif
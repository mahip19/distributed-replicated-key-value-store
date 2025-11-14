#ifndef __SERVERSOCKET_H__
#define __SERVERSOCKET_H__

#include <memory>

#include "Socket.h"

class ServerSocket: public Socket {
public:
	ServerSocket() {}
	~ServerSocket() {}

	ServerSocket(int fd, bool nagle_on = NAGLE_ON);

	bool Init(int port);
	bool Init(const std::string &ip, int port);  // for outgoing connections (PFA → peer)

	std::unique_ptr<ServerSocket> Accept();
};


#endif // end of #ifndef __SERVERSOCKET_H__

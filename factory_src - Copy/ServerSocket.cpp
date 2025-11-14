#include <iostream>

#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <unistd.h>


#include "ServerSocket.h"

ServerSocket::ServerSocket(int fd, bool nagle_on) {
	fd_ = fd;
	is_initialized_ = true;
	NagleOn(nagle_on);
}

bool ServerSocket::Init(int port) {
	if (is_initialized_) {
		return true;
	}

	struct sockaddr_in addr;
	fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_ < 0) {
		perror("ERROR: failed to create a socket");
		return false;
	}

	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if ((bind(fd_, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
		perror("ERROR: failed to bind");
		return false;
	}

	listen(fd_, 256);

	is_initialized_ = true;
	return true;
}


bool ServerSocket::Init(const std::string &ip, int port) {
    if (is_initialized_) {
        return true;
    }

    struct sockaddr_in serv_addr;
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        perror("[PFA] ERROR: failed to create socket");
        return false;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "[PFA] ERROR: Invalid IP " << ip << std::endl;
        close(fd_);
        return false;
    }

    if (connect(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[PFA] ERROR: connection failed");
        close(fd_);
        return false;
    }

    is_initialized_ = true;
    // std::cout << "[PFA] Connected to peer " << ip << ":" << port << std::endl;
    return true;
}


std::unique_ptr<ServerSocket> ServerSocket::Accept() {
	int accepted_fd;
	struct sockaddr_in addr;
	unsigned int addr_size = sizeof(addr);
	accepted_fd = accept(fd_, (struct sockaddr *) &addr, &addr_size);
	if (accepted_fd < 0) {
		perror("ERROR: failed to accept connection");
		return nullptr;
	}

	return std::unique_ptr<ServerSocket>(new ServerSocket(accepted_fd, IsNagleOn()));
}

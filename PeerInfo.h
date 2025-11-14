#ifndef __PEERINFO_H__
#define __PEERINFO_H__

#include <string>

struct PeerInfo {
    int id;
    std::string ip;
    int port;
};

#endif

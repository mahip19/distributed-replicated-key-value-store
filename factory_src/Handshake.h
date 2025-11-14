#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#include <cstring>
#include <string>

struct Handshake {
    char tag[8];   // "CLIENT" or "PFA" (null-terminated)

    Handshake() { std::memset(tag, 0, sizeof(tag)); }

    // Convenience helper
    explicit Handshake(const std::string& role) {
        std::memset(tag, 0, sizeof(tag));
        std::strncpy(tag, role.c_str(), sizeof(tag) - 1);
    }
};

#endif // __HANDSHAKE_H__

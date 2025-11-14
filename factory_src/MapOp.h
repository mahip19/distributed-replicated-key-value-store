#ifndef __MAPOP_H__
#define __MAPOP_H__

#include <cstring>
#include <arpa/inet.h>

struct MapOp {
    int opcode;  //  1 = write/update
    int arg1;    //  customer_id
    int arg2;    //  new_order

    int Size() const {
        return sizeof(opcode) + sizeof(arg1) + sizeof(arg2);
    }

    void Marshal(char* buffer) const {
        int offset = 0;
        int net_opcode = htonl(opcode);
        int net_arg1   = htonl(arg1);
        int net_arg2   = htonl(arg2);
        memcpy(buffer + offset, &net_opcode, sizeof(net_opcode));
        offset += sizeof(net_opcode);
        memcpy(buffer + offset, &net_arg1, sizeof(net_arg1));
        offset += sizeof(net_arg1);
        memcpy(buffer + offset, &net_arg2, sizeof(net_arg2));
    }

    void Unmarshal(const char* buffer) {
        int offset = 0;
        int net_opcode, net_arg1, net_arg2;
        memcpy(&net_opcode, buffer + offset, sizeof(net_opcode));
        offset += sizeof(net_opcode);
        memcpy(&net_arg1, buffer + offset, sizeof(net_arg1));
        offset += sizeof(net_arg1);
        memcpy(&net_arg2, buffer + offset, sizeof(net_arg2));
        opcode = ntohl(net_opcode);
        arg1   = ntohl(net_arg1);
        arg2   = ntohl(net_arg2);
    }
};

#endif // __MAPOP_H__

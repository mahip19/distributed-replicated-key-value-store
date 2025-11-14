#include "ClientStub.h"
#include "Handshake.h"


ClientStub::ClientStub() {}

bool ClientStub::Init(std::string ip, int port) {
    bool ok = socket.Init(ip, port);
    if (!ok) {
        std::cerr << "[ClientStub] Connection to " << ip << ":" << port << " failed\n";
        return false;
    }

    // Send exactly 8 bytes for alignment
    char tag[8] = {'C', 'L', 'I', 'E', 'N', 'T', '\0', '\0'};
    socket.Send(tag, 8, 0);

    // std::cout << "[ClientStub] Connected to server as CLIENT (" << ip << ":" << port << ")\n";
    return true;
}


RobotInfo ClientStub::Order(RobotOrder order) {
    RobotInfo info;
    char buffer[32];
    int size;
    order.Marshal(buffer);
    size = order.Size();
    
    //std::cout << "[ClientStub::Order] Sending " << size << " bytes" << std::endl;
    
    if (socket.Send(buffer, size, 0)) {
        //std::cout << "[ClientStub::Order] Send successful, waiting for reply..." << std::endl;
        size = info.Size();
        if (socket.Recv(buffer, size, 0)) {
            info.Unmarshal(buffer);
        } 
    }
    return info;
}

CustomerRecord ClientStub::ReadRecord(RobotOrder req) {
    CustomerRecord record;
    char buffer[32];

    req.Marshal(buffer);
    if (socket.Send(buffer, req.Size(), 0)) {
        if (socket.Recv(buffer, record.Size(), 0)) {
            record.Unmarshal(buffer);
        }
    }
    return record;
}


#include "FactoryStub.h"
#include <cstring>

FactoryStub::FactoryStub() {}

bool FactoryStub::Init(std::unique_ptr<ServerSocket> sock) {
    socket = std::move(sock);
    return socket != nullptr;
}

int FactoryStub::SendReplication(const ReplicationRequest& req) {
    char buffer[sizeof(ReplicationRequest)];
    req.Marshal(buffer);
    return socket->Send(buffer, req.Size(), 0);
}

ReplicationRequest FactoryStub::ReceiveReplication() {
    char buffer[sizeof(ReplicationRequest)];
    socket->Recv(buffer, sizeof(buffer), 0);
    ReplicationRequest req;
    req.Unmarshal(buffer);
    return req;
}

int FactoryStub::SendAck(const ReplicationAck& ack) {
    char buffer[sizeof(ReplicationAck)];
    ack.Marshal(buffer);
    return socket->Send(buffer, ack.Size(), 0);
}

bool FactoryStub::ReceiveReplicationSafe(ReplicationRequest& req) {
    char buffer[sizeof(ReplicationRequest)];
    
    // if recv succeeds
    if (!socket->Recv(buffer, sizeof(buffer), 0)) {
        return false;  //  failed
    }
    
    req.Unmarshal(buffer);
    return true;
}

ReplicationAck FactoryStub::ReceiveAck() {
    char buffer[sizeof(ReplicationAck)];
    socket->Recv(buffer, sizeof(buffer), 0);
    ReplicationAck ack;
    ack.Unmarshal(buffer);
    return ack;
}

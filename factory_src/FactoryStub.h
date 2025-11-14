#ifndef __FACTORY_STUB_H__
#define __FACTORY_STUB_H__

#include "ServerSocket.h"
#include "Messages.h"

class FactoryStub {
public:
    FactoryStub();
    bool Init(std::unique_ptr<ServerSocket> sock);

    // send/receive replication requests
    int SendReplication(const ReplicationRequest& req);
    ReplicationRequest ReceiveReplication();

    // send/receive acks
    int SendAck(const ReplicationAck& ack);
    ReplicationAck ReceiveAck();
    bool ReceiveReplicationSafe(ReplicationRequest& req);


private:
    std::unique_ptr<ServerSocket> socket;
};

#endif

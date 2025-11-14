#include "ServerStub.h"

ServerStub::ServerStub() {}

void ServerStub::Init(std::unique_ptr<ServerSocket> socket) {
	this->socket = std::move(socket);
}

RobotOrder ServerStub::ReceiveRequest() {
    char buffer[32];
    memset(buffer, 0, 32);  // Clear buffer first
    
    RobotOrder order;
    int size_to_read = order.Size();
    //std::cout << "[ServerStub] About to recv " << size_to_read << " bytes" << std::endl;
    
    if (socket->Recv(buffer, size_to_read, 0)) {
        // std::cout << "[ServerStub] Received bytes: ";
        // for (int i = 0; i < size_to_read; i++) {
        //     printf("%02x ", (unsigned char)buffer[i]);
        // }
        // std::cout << std::endl;
        
        order.Unmarshal(buffer);
    } else {
        //std::cout << "[ServerStub] Recv failed!" << std::endl;
    }
    return order;	
}

int ServerStub::ShipRobot(RobotInfo info) {
	char buffer[32];
	info.Marshal(buffer);
	return socket->Send(buffer, info.Size(), 0);
}

int ServerStub::ReturnRecord(CustomerRecord record) {
	char buffer[32];
	record.Marshal(buffer);
	return socket->Send(buffer, record.Size(), 0);	
}


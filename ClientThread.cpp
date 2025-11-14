#include "ClientThread.h"
#include "Messages.h"

#include <iostream>

ClientThreadClass::ClientThreadClass() {}

void ClientThreadClass::ThreadBody(std::string ip, int port, int id, int orders, int type) {
	customer_id = id;
	num_orders = orders;
	request_type = type;
	if (!stub.Init(ip, port)) {
		// std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
		return;	
	}
	// ---- WRITE(type = 1)
    if (request_type == 1) {
        for (int i = 0; i < num_orders; i++) {
            RobotOrder order;
            order.SetOrder(customer_id, i, request_type);

            timer.Start();
            RobotInfo reply = stub.Order(order);
            timer.EndAndMerge();

            if (!reply.IsValid()) {
                std::cout << "Invalid reply for customer " << customer_id << std::endl;
                break;
            }
        }
    }

    // ---- READ(type = 2)
    else if (request_type == 2) {
        for (int i = 0; i < num_orders; i++) {
            RobotOrder order;
            order.SetOrder(customer_id, i, request_type);
            timer.Start();  // ADD THIS - measure read performance
            CustomerRecord rec = stub.ReadRecord(order);
            timer.EndAndMerge();  // ADD THIS
        }
    }

    // ---- PRINT(type = 3)
    else if (request_type == 3) {
        // iterate through all possible ids
        for (int cid = 0; cid < num_orders; cid++) {
            RobotOrder order;
            order.SetOrder(cid, -1, 2);   // reads each record
            CustomerRecord rec = stub.ReadRecord(order);
            if (rec.GetCustomerId() != -1)
                rec.Print();
        }
    }

    else {
        //std::cout << "Unknown request type: " << request_type << std::endl;
    }
}

ClientTimer ClientThreadClass::GetTimer() {
	return timer;	
}


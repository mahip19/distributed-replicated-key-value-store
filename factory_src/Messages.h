#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>
#include <iostream>  
#include <cstring>    
#include <arpa/inet.h> 
#include <mutex>
#include "MapOp.h"

// ------------------- Customer Request -------------------

class RobotOrder
{
private:
	int customer_id;
	int order_number;
	int request_type; // 1 = write (order), 2 = read, 3 = print-scan

public:
	RobotOrder();
	void operator=(const RobotOrder &order)
	{
		customer_id = order.customer_id;
		order_number = order.order_number;
		request_type = order.request_type;
	}

	void SetOrder(int cid, int order_num, int type);
	int GetCustomerId();
	int GetOrderNumber();
	int GetRequestType();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

// ------------------- Customer Record / Reply 

class RobotInfo
{
private:
	int customer_id;
	int order_number;
	int request_type; // same field reused for consistency
	int engineer_id;
	int admin_id; // replaces expert_id

public:
	RobotInfo();
	void operator=(const RobotInfo &info)
	{
		customer_id = info.customer_id;
		order_number = info.order_number;
		request_type = info.request_type;
		engineer_id = info.engineer_id;
		admin_id = info.admin_id;
	}

	void SetInfo(int cid, int order_num, int type, int engid, int admid);
	void CopyOrder(RobotOrder order);
	void SetEngineerId(int id);
	void SetAdminId(int id);

	int GetCustomerId();
	int GetOrderNumber();
	int GetRequestType();
	int GetEngineerId();
	int GetAdminId();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

struct CustomerRecord {
    int customer_id;
    int last_order;

    CustomerRecord() {
        customer_id = -1;
        last_order  = -1;
    }

    int Size() {
        return sizeof(customer_id) + sizeof(last_order);
    }
	void SetRecord(int cid, int last) {
        customer_id = cid;
        last_order  = last;
    }

    int GetCustomerId() { return customer_id; }
    int GetLastOrder()  { return last_order; }


    void Marshal(char *buffer) {
        int net_id  = htonl(customer_id);
        int net_ord = htonl(last_order);
        int offset  = 0;
        memcpy(buffer + offset, &net_id, sizeof(net_id)); offset += sizeof(net_id);
        memcpy(buffer + offset, &net_ord, sizeof(net_ord));
    }

    void Unmarshal(char *buffer) {
        int net_id, net_ord;
        int offset = 0;
        memcpy(&net_id,  buffer + offset, sizeof(net_id)); offset += sizeof(net_id);
        memcpy(&net_ord, buffer + offset, sizeof(net_ord));
        customer_id = ntohl(net_id);
        last_order  = ntohl(net_ord);
    }

    void Print() {
        if (customer_id != -1) {
			static std::mutex cout_lock;
			std::lock_guard<std::mutex> guard(cout_lock);
			std::cout << customer_id << "\t" << last_order << std::endl;
    	}
    }
};

// ----- REPLICATION messages for PFA ----> IFA
struct ReplicationRequest {
	int primary_id;        // ID of the primary sending this message
    int committed_index;   // up to which index is committed
    int last_index;        // index of the new operation to append
    MapOp op;              // the operation itself (reuse from smr_log)

    int Size() const {
        return sizeof(primary_id) + sizeof(committed_index) + sizeof(last_index)
               + sizeof(op);
    }

    void Marshal(char* buffer) const {
        int offset = 0;
        int net_primary_id     = htonl(primary_id);
        int net_committed_idx  = htonl(committed_index);
        int net_last_idx       = htonl(last_index);

        memcpy(buffer + offset, &net_primary_id, sizeof(net_primary_id));
        offset += sizeof(net_primary_id);
        memcpy(buffer + offset, &net_committed_idx, sizeof(net_committed_idx));
        offset += sizeof(net_committed_idx);
        memcpy(buffer + offset, &net_last_idx, sizeof(net_last_idx));
        offset += sizeof(net_last_idx);

        memcpy(buffer + offset, &op, sizeof(op)); // MapOp is plain ints
    }

    void Unmarshal(const char* buffer) {
        int offset = 0;
        int net_primary_id, net_committed_idx, net_last_idx;

        memcpy(&net_primary_id, buffer + offset, sizeof(net_primary_id));
        offset += sizeof(net_primary_id);
        memcpy(&net_committed_idx, buffer + offset, sizeof(net_committed_idx));
        offset += sizeof(net_committed_idx);
        memcpy(&net_last_idx, buffer + offset, sizeof(net_last_idx));
        offset += sizeof(net_last_idx);

        primary_id      = ntohl(net_primary_id);
        committed_index = ntohl(net_committed_idx);
        last_index      = ntohl(net_last_idx);

        memcpy(&op, buffer + offset, sizeof(op));
    }
};

// ------ acknowledgment from IFA back to PFA
struct ReplicationAck {
    int replica_id;
    int last_index;

    int Size() const { return sizeof(replica_id) + sizeof(last_index); }

    void Marshal(char* buffer) const {
        int net_replica_id = htonl(replica_id);
        int net_last_index = htonl(last_index);
        int offset = 0;
        memcpy(buffer + offset, &net_replica_id, sizeof(net_replica_id));
        offset += sizeof(net_replica_id);
        memcpy(buffer + offset, &net_last_index, sizeof(net_last_index));
    }

    void Unmarshal(const char* buffer) {
        int net_replica_id, net_last_index;
        int offset = 0;
        memcpy(&net_replica_id, buffer + offset, sizeof(net_replica_id));
        offset += sizeof(net_replica_id);
        memcpy(&net_last_index, buffer + offset, sizeof(net_last_index));
        replica_id = ntohl(net_replica_id);
        last_index = ntohl(net_last_index);
    }
};

#endif // __MESSAGES_H__

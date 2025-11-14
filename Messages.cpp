#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include "Messages.h"

RobotOrder::RobotOrder()
{
	customer_id = -1;
	order_number = -1;
	request_type = -1;
}

void RobotOrder::SetOrder(int cid, int number, int type)
{
	customer_id = cid;
	order_number = number;
	request_type = type;
}

int RobotOrder::GetCustomerId() { return customer_id; }
int RobotOrder::GetOrderNumber() { return order_number; }
int RobotOrder::GetRequestType() { return request_type; }

int RobotOrder::Size()
{
	return sizeof(customer_id) + sizeof(order_number) + sizeof(request_type);
}

void RobotOrder::Marshal(char *buffer)
{
    // std::cout << "[DEBUG Marshal] BEFORE: cid=" << customer_id 
    //           << " order=" << order_number << " type=" << request_type << std::endl;
    
    int net_customer_id = htonl(customer_id);
    int net_order_number = htonl(order_number);
    int net_request_type = htonl(request_type);
    int offset = 0;

    memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
    offset += sizeof(net_customer_id);
    memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
    offset += sizeof(net_order_number);
    memcpy(buffer + offset, &net_request_type, sizeof(net_request_type));
    
    //std::cout << "[DEBUG Marshal] Raw bytes sent: ";
    // for (int i = 0; i < 12; i++) {
    //     printf("%02x ", (unsigned char)buffer[i]);
    // }
    // std::cout << std::endl;
}
void RobotOrder::Unmarshal(char *buffer)
{
    // debugging Print raw bytes
    // std::cout << "[DEBUG Unmarshal] Raw bytes: ";
    // for (int i = 0; i < 12; i++) {
    //     printf("%02x ", (unsigned char)buffer[i]);
    // }
    // std::cout << std::endl;
    
    int net_customer_id, net_order_number, net_request_type;
    int offset = 0;

    memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
    offset += sizeof(net_customer_id);
    memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
    offset += sizeof(net_order_number);
    memcpy(&net_request_type, buffer + offset, sizeof(net_request_type));

    customer_id = ntohl(net_customer_id);
    order_number = ntohl(net_order_number);
    request_type = ntohl(net_request_type);
    
	// std::cout << "[DEBUG Unmarshal] After ntohl: cid=" << customer_id 
	//           << " order=" << order_number << " type=" << request_type << std::endl;
}

bool RobotOrder::IsValid()
{
	return (customer_id != -1);
}

void RobotOrder::Print()
{
	std::cout << "id " << customer_id
			  << " num " << order_number
			  << " type " << request_type
			  << std::endl;
}

// ------------------------- RobotInfo -------------------------

RobotInfo::RobotInfo()
{
	customer_id = -1;
	order_number = -1;
	request_type = -1;
	engineer_id = -1;
	admin_id = -1;
}

void RobotInfo::SetInfo(int cid, int number, int type, int engid, int admid)
{
	customer_id = cid;
	order_number = number;
	request_type = type;
	engineer_id = engid;
	admin_id = admid;
}

void RobotInfo::CopyOrder(RobotOrder order)
{
	customer_id = order.GetCustomerId();
	order_number = order.GetOrderNumber();
	request_type = order.GetRequestType();
}

void RobotInfo::SetEngineerId(int id) { engineer_id = id; }
void RobotInfo::SetAdminId(int id) { admin_id = id; }

int RobotInfo::GetCustomerId() { return customer_id; }
int RobotInfo::GetOrderNumber() { return order_number; }
int RobotInfo::GetRequestType() { return request_type; }
int RobotInfo::GetEngineerId() { return engineer_id; }
int RobotInfo::GetAdminId() { return admin_id; }

int RobotInfo::Size()
{
	return sizeof(customer_id) + sizeof(order_number) + sizeof(request_type) + sizeof(engineer_id) + sizeof(admin_id);
}

void RobotInfo::Marshal(char *buffer)
{
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_request_type = htonl(request_type);
	int net_engineer_id = htonl(engineer_id);
	int net_admin_id = htonl(admin_id);
	int offset = 0;

	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_request_type, sizeof(net_request_type));
	offset += sizeof(net_request_type);
	memcpy(buffer + offset, &net_engineer_id, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(buffer + offset, &net_admin_id, sizeof(net_admin_id));
}

void RobotInfo::Unmarshal(char *buffer)
{
	int net_customer_id, net_order_number, net_request_type;
	int net_engineer_id, net_admin_id;
	int offset = 0;

	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_request_type, buffer + offset, sizeof(net_request_type));
	offset += sizeof(net_request_type);
	memcpy(&net_engineer_id, buffer + offset, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(&net_admin_id, buffer + offset, sizeof(net_admin_id));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	request_type = ntohl(net_request_type);
	engineer_id = ntohl(net_engineer_id);
	admin_id = ntohl(net_admin_id);
}

bool RobotInfo::IsValid()
{
	return (customer_id != -1);
}

void RobotInfo::Print()
{
	std::cout << "id " << customer_id
			  << " num " << order_number
			  << " type " << request_type
			  << " engid " << engineer_id
			  << " adminid " << admin_id
			  << std::endl;
}

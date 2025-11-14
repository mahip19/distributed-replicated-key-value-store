#include <iostream>
#include <memory>
#include <cstring>

#include "ServerThread.h"
#include "ServerStub.h"
#include "FactoryStub.h"

void RobotFactory::Init(int id, std::vector<PeerInfo> &peers_list)
{
    factory_id = id;
    peers = peers_list;

    last_index = -1;
    committed_index = -1;
    primary_id = -1;

    // std::cout << "[Init] Factory " << factory_id
    //           << " initialized with " << peers.size() << " peers." << std::endl;
}

void RobotFactory::EngineerThread(std::unique_ptr<ServerSocket> socket, int id)
{
    char tag_buffer[8] = {0};

    // Read 8 bytes for identification
    if (!socket->Recv(tag_buffer, 8, 0))
    {
        // std::cerr << "[Engineer] Failed to read identification tag" << std::endl;
        return;
    }

    // Check connection type
    if (strncmp(tag_buffer, "PFA", 3) == 0)
    {
        // std::cout << "[Engineer] Connection from PFA (replication link)" << std::endl;
        HandleReplication(std::move(socket));
        return;
    }
    else if (strncmp(tag_buffer, "CLIENT", 6) == 0)
    {
        //  std::cout << "[Engineer] Connection from CLIENT" << std::endl;
    }
    else
    {
        // std::cerr << "[Engineer] Unknown tag: [" << tag_buffer << "]" << std::endl;
        return;
    }

    // customer-handling logic
    int engineer_id = id;
    RobotOrder order;
    ServerStub stub;
    stub.Init(std::move(socket));

    while (true)
    {
        // std::cout << "[Engineer " << engineer_id << "] Waiting for request..." << std::endl;
        order = stub.ReceiveRequest();
        if (!order.IsValid())
        {
            // std::cout << "[Engineer " << engineer_id << "] Invalid order, disconnecting" << std::endl;
            break;
        }

        int cid = order.GetCustomerId();
        int order_num = order.GetOrderNumber();
        int type = order.GetRequestType();

        // std::cout << "[Engineer " << engineer_id << "] Received request: cid=" << cid
        //           << " order=" << order_num << " type=" << type << std::endl;

        if (type == 1)
        {
            // std::cout << "[Engineer " << engineer_id << "] Sending request to admin for customer "
            //           << cid << " order " << order_num << std::endl;

            // Robot order - needs admin to update record
            std::unique_ptr<AdminRequest> admin_req(new AdminRequest());
            admin_req->customer_id = cid;
            admin_req->new_order = order_num;
            std::future<int> fut = admin_req->prom.get_future();

            {
                std::lock_guard<std::mutex> lock(admin_q_lock);
                admin_q.push(std::move(admin_req));
                // std::cout << "[Engineer " << engineer_id << "] Added to admin queue, size="
                //           << admin_q.size() << std::endl;
            }
            admin_cv.notify_one();

            // std::cout << "[Engineer " << engineer_id << "] Waiting for admin response..." << std::endl;

            // Wait for admin to complete
            (void)fut.get();

            // std::cout << "[Engineer " << engineer_id << "] Received admin response!" << std::endl;

            RobotInfo reply;
            reply.CopyOrder(order);
            reply.SetEngineerId(engineer_id);
            reply.SetAdminId(0);

            // std::cout << "[Engineer " << engineer_id << "] Sending reply to client" << std::endl;
            stub.ShipRobot(reply);
            // std::cout << "[Engineer " << engineer_id << "] Reply sent!" << std::endl;
        }
        else if (type == 2)
        {
            // std::cout << "[Engineer " << engineer_id << "] Processing read request for customer "
            //           << cid << std::endl;
            // Read record request
            int last_order = -1;
            {
                std::lock_guard<std::mutex> lock(record_lock);
                auto it = customer_record.find(cid);
                if (it != customer_record.end())
                    last_order = it->second;
            }
            CustomerRecord record;
            record.SetRecord(cid, last_order);
            stub.ReturnRecord(record);
            // std::cout << "[Engineer " << engineer_id << "] Read response sent" << std::endl;
        }
    }

    // std::cout << "[Engineer] Client disconnected" << std::endl;
}

// Invoked when a PFA connects to send replication messages to this backup (IFA).
void RobotFactory::HandleReplication(std::unique_ptr<ServerSocket> socket)
{
    FactoryStub stub;
    stub.Init(std::move(socket));

    // std::cout << "[IFA] Starting replication handler" << std::endl;

    while (true)
    {
        // std::cout << "[IFA DEBUG] Waiting for replication request..." << std::endl;

        ReplicationRequest req;
        if (!stub.ReceiveReplicationSafe(req))
        {
            //  std::cout << "[IFA] Primary connection lost" << std::endl;
            break;
        }

        // std::cout << "[IFA DEBUG] Processing replication..." << std::endl;

        {
            std::lock_guard<std::mutex> lock1(log_lock);
            std::lock_guard<std::mutex> lock2(record_lock);

            if (primary_id != req.primary_id)
            {
                // std::cout << "[IFA] Setting primary_id to " << req.primary_id << std::endl;
                primary_id = req.primary_id;
            }

            last_index = req.last_index;
            if ((int)smr_log.size() <= last_index)
            {
                smr_log.resize(last_index + 1);
            }
            smr_log[last_index] = req.op;

            for (int i = committed_index + 1; i <= req.committed_index; ++i)
            {
                if (i >= 0 && i < (int)smr_log.size())
                {
                    MapOp &op = smr_log[i];
                    if (op.opcode == 1)
                    {
                        customer_record[op.arg1] = op.arg2;
                    }
                }
            }
            committed_index = req.committed_index;
        }

        // std::cout << "[IFA DEBUG] Sending ack..." << std::endl;

        ReplicationAck ack;
        ack.replica_id = factory_id;
        ack.last_index = last_index;
        // stub.SendAck(ack);

        if (!stub.SendAck(ack))
        {
            //  std::cout << "[IFA] Failed to send ack, primary likely dead" << std::endl;
            break;
        }

        //  std::cout << "[IFA] Replicated log idx=" << last_index
        //          << " committed=" << committed_index << std::endl;
    }

    //  std::cout << "[IFA] Replication handler terminated" << std::endl;

    {
        std::lock_guard<std::mutex> lock(log_lock);
        primary_id = -1;
        // std::cout << "[IFA] Reset primary_id to -1" << std::endl;
    }
}

void RobotFactory::PrintReplicationState()
{
    std::lock_guard<std::mutex> lock(log_lock);
    std::cout << "[State] Factory " << factory_id
              << " | primary=" << primary_id
              << " | last=" << last_index
              << " | committed=" << committed_index
              << std::endl;
}

// Creates a new socket and sends the role tag "PFA" (padded to 6 bytes).
std::unique_ptr<ServerSocket> RobotFactory::ConnectToPeer(const PeerInfo &peer)
{
    std::unique_ptr<ServerSocket> sock(new ServerSocket());
    if (!sock->Init(peer.ip, peer.port))
    {
        // std::cout << "[PFA] Failed to connect to peer " << peer.id << std::endl;
        return nullptr;
    }

    // Send PFA tag padded to 6 bytes
    char tag[8] = {'P', 'F', 'A', '\0', '\0', '\0', '\0', '\0'};
    if (!sock->Send(tag, 8, 0))
    {
        // std::cout << "[PFA] Failed to send PFA tag to peer " << peer.id << std::endl;
        return nullptr;
    }

    // std::cout << "[PFA] Connected to peer " << peer.id
    //           << " (" << peer.ip << ":" << peer.port << ")" << std::endl;
    return sock;
}

void RobotFactory::AdminThread(int id)
{
    // std::cout << "[AdminThread] Started (PFA role)" << std::endl;

    while (true)
    {
        std::unique_ptr<AdminRequest> req;

        // Wait for request from engineer
        {
            std::unique_lock<std::mutex> lock(admin_q_lock);
            admin_cv.wait(lock, [this]()
                          { return !admin_q.empty(); });
            req = std::move(admin_q.front());
            admin_q.pop();
        }

        // Create MapOp
        MapOp op;
        op.opcode = 1;
        op.arg1 = req->customer_id;
        op.arg2 = req->new_order;

        // Append to log and update last_index
        {
            std::lock_guard<std::mutex> lock(log_lock);

            // If this is the first request, become primary
            if (primary_id != factory_id)
            {
                // std::cout << "[PFA] Becoming primary (factory_id=" << factory_id << ")" << std::endl;
                primary_id = factory_id;

                // Connect to all peers (only if there are peers)
                if (!peers.empty())
                {
                    ConnectToAllPeers();
                }
            }

            last_index++;
            if ((int)smr_log.size() <= last_index)
            {
                smr_log.resize(last_index + 1);
            }
            smr_log[last_index] = op;
        }

        // Replicate to all backups (only if there are peers)
        if (!peers.empty())
        {
            ReplicateToBackups();
        }

        // Apply the operation to local customer_record and commit
        // ALSO apply any previously uncommitted entries!
        {
            std::lock_guard<std::mutex> lock1(log_lock);
            std::lock_guard<std::mutex> lock2(record_lock);

            // Apply all uncommitted entries from committed_index+1 to last_index
            for (int i = committed_index + 1; i <= last_index; ++i)
            {
                if (i >= 0 && i < (int)smr_log.size())
                {
                    MapOp &entry = smr_log[i];
                    if (entry.opcode == 1)
                    {
                        customer_record[entry.arg1] = entry.arg2;
                        // std::cout << "[PFA DEBUG] Applied log[" << i << "]: customer "
                        //           << entry.arg1 << " → order " << entry.arg2 << std::endl;
                    }
                }
            }
            committed_index = last_index;
        }

        // std::cout << "[PFA] Committed log idx=" << last_index << std::endl;

        // Notify engineer that update is complete
        req->prom.set_value(1);
    }
}

void RobotFactory::ConnectToAllPeers()
{
    // std::cout << "[PFA] Connecting to all peers..." << std::endl;
    peer_stubs.clear();

    for (auto &peer : peers)
    {
        auto sock = ConnectToPeer(peer);
        if (sock)
        {
            std::unique_ptr<FactoryStub> stub(new FactoryStub());
            stub->Init(std::move(sock));
            peer_stubs.push_back(std::move(stub));
            // std::cout << "[PFA] Successfully connected to peer " << peer.id << std::endl;
        }
        else
        {
            // std::cout << "[PFA] Failed to connect to peer " << peer.id << std::endl;
            peer_stubs.push_back(nullptr);
        }
    }
}

void RobotFactory::ReplicateToBackups()
{
    std::lock_guard<std::mutex> lock(log_lock);

    // std::cout << "[PFA DEBUG] Starting replication to " << peer_stubs.size() << " peers" << std::endl;

    for (size_t i = 0; i < peer_stubs.size(); ++i)
    {
        if (!peer_stubs[i])
        {
            // std::cout << "[PFA DEBUG] Peer " << i << " is null, skipping" << std::endl;
            continue;
        }

        // std::cout << "[PFA DEBUG] Replicating to peer " << i << std::endl;

        ReplicationRequest req;
        req.primary_id = factory_id;
        req.committed_index = committed_index;
        req.last_index = last_index;
        req.op = smr_log[last_index];

        if (!peer_stubs[i]->SendReplication(req))
        {
            //  std::cout << "[PFA] Failed to send replication to peer " << i << std::endl;
            peer_stubs[i] = nullptr;
            continue;
        }

        // std::cout << "[PFA DEBUG] Waiting for ack from peer " << i << std::endl;

        ReplicationAck ack = peer_stubs[i]->ReceiveAck();

        // std::cout << "[PFA] Received ack from peer " << ack.replica_id
        //           << " (last_index=" << ack.last_index << ")" << std::endl;  // UNCOMMENT THIS
    }

    // std::cout << "[PFA DEBUG] Replication complete" << std::endl;
}
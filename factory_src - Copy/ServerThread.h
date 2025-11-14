#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <map>
#include <vector>
#include <memory>

#include "Messages.h"
#include "ServerSocket.h"
#include "PeerInfo.h"
#include "MapOp.h"
#include "FactoryStub.h"  // ADD THIS LINE - full include instead of forward declaration

struct AdminRequest
{
	int customer_id;
	int new_order;
	std::promise<int> prom;
};

class RobotFactory
{
private:
	std::queue<std::unique_ptr<AdminRequest>> admin_q;
	std::mutex admin_q_lock;
	std::condition_variable admin_cv;

	// unique factory id
	int factory_id;
	// list of its peer factories
	std::vector<PeerInfo> peers;

	// Connection stubs to peer factories (for PFA role)
	std::vector<std::unique_ptr<FactoryStub>> peer_stubs;

	// vars to keep track for replication
	int last_index;         // last written index in smr log
	int committed_index;    // highest committed index applied to map
	int primary_id;         // current production factory (-1 = none)

	// shared log map and vector
	std::map<int, int> customer_record;
	std::mutex record_lock;
	
	// state machine log
	std::vector<MapOp> smr_log;
	std::mutex log_lock;

	// Helper methods for replication
	void ConnectToAllPeers();
	void ReplicateToBackups();

	RobotInfo CreateRegularRobot(RobotOrder order, int engineer_id);
	RobotInfo CreateSpecialRobot(RobotOrder order, int engineer_id);

public:
	void Init(int id, std::vector<PeerInfo>& peers_list);
	void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
	void AdminThread(int id);
	void PrintReplicationState(); // to know the current replication status
	void HandleReplication(std::unique_ptr<ServerSocket> socket);
	std::unique_ptr<ServerSocket> ConnectToPeer(const PeerInfo& peer);
};

#endif // end of #ifndef __SERVERTHREAD_H__
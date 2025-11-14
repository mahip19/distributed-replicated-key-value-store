#include <vector>
#include <vector>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "ServerSocket.h"
#include "ServerThread.h"
#include "PeerInfo.h"

int main(int argc, char *argv[])
{
	int port;
	int engineer_cnt = 0;
	
	ServerSocket socket;
	RobotFactory factory;
	std::unique_ptr<ServerSocket> new_socket;
	std::vector<std::thread> thread_vector;

	
	// expected cmd: ./server port uniqId peers (id ip port)*peers
	if (argc < 4) {
		std::cout << "Usage: " << argv[0]
              << " [port #] [factory_id] [num_peers] "
              << "(repeat [peer_id peer_ip peer_port])" << std::endl;
    	return 0;
	}

	port = atoi(argv[1]);
	int factory_id = atoi(argv[2]);
	int num_peers = atoi(argv[3]);

	if (num_peers * 3 + 4 != argc) {
		std::cout << "Error: expected " << num_peers
              << " peer entries (id ip port)." << std::endl;
    	return 0;
	}

	std::vector<PeerInfo> peers;
	int offset = 0;
	for (int i = 0; i < num_peers; i++) {
		offset = i*3 + 4;
		PeerInfo p;
		p.id = atoi(argv[offset]);
		p.ip = argv[offset+1];
		p.port = atoi(argv[offset+2]);
		peers.push_back(p);
	}

	// std::cout << "Factory ID: " << factory_id << std::endl;
	// for (auto &p : peers)
    // {
	// 	std::cout << "Peer " << p.id << ": " << p.ip << ":" << p.port << std::endl;
	// }

	// initializing factory so that this factory knows its peers info as well
	factory.Init(factory_id, peers);
	
	if (!socket.Init(port))
	{
		std::cout << "Socket initialization failed" << std::endl;
		return 0;
	}
	// std::cout << "[ServerMain] Listening on port " << port << std::endl;

	// now we just need one admin
	std::thread admin_thread(&RobotFactory::AdminThread, &factory, 0);
	thread_vector.push_back(std::move(admin_thread));

	

	while ((new_socket = socket.Accept()))
	{
		std::thread engineer_thread(&RobotFactory::EngineerThread, &factory,
									std::move(new_socket), engineer_cnt++);
		thread_vector.push_back(std::move(engineer_thread));
	}


	for (auto &th : thread_vector) {
        if (th.joinable()) th.join();
    }
	return 0;
}

# Distributed Replicated Key-Value Store

A fault-tolerant distributed key-value store implementing primary-backup chain replication for CS 6650 (Building Scalable Distributed Systems).

## 🎯 Overview

This project implements a replicated in-memory key-value store across multiple servers using state machine replication and primary-backup protocol. The system maintains customer records for a simulated robot factory environment with automatic failover and fault tolerance.

## ✨ Features

- **State Machine Replication**: Ordered log of operations ensures consistency across replicas
- **Primary-Backup Protocol**: Chain replication with sequential acknowledgments
- **Fault Tolerance**: Graceful handling of primary and backup failures
- **Automatic Failover**: Any backup can become primary when current primary fails
- **Multi-threaded Architecture**: Thread-per-connection model with synchronized access
- **Performance Monitoring**: Built-in latency and throughput measurement

## 🏗️ Architecture

### Server Roles

1. **Engineer Thread**: Handles client connections, delegates writes to admin, serves reads directly
2. **Production Factory Admin (PFA)**: Replicates operations from primary to all backups sequentially
3. **Idle Factory Admin (IFA)**: Receives and applies replicated operations at backup servers

### Key Components

- **State Machine Log** (`std::vector<MapOp>`): Ordered sequence of all operations
- **Customer Record** (`std::map<int,int>`): Derived state mapping customer_id → last_order_number
- **Replication Variables**: `last_index`, `committed_index`, `primary_id` for coordination

### Consistency Model

- Operations append to log, replicate to backups, then commit
- Primary maintains one additional committed entry vs backups (by design)
- Backups apply only committed entries to ensure consistency

## 🔧 Building

### Prerequisites

- C++11 or higher
- Linux/Unix environment
- Standard build tools (g++, make)

### Compilation
```bash
make
```

This produces two binaries:
- `server` - Factory server program
- `client` - Client program for testing

### Clean Build
```bash
make clean
make
```

## 🚀 Usage

### Server
```bash
./server [port] [factory_id] [num_peers] (repeat [peer_id peer_ip peer_port])
```

**Examples:**

Single server (no replication):
```bash
./server 5000 0 0
```

Three-server cluster:
```bash
# Terminal 1
./server 5000 0 2 1 127.0.0.1 5001 2 127.0.0.1 5002

# Terminal 2
./server 5001 1 2 0 127.0.0.1 5000 2 127.0.0.1 5002

# Terminal 3
./server 5002 2 2 0 127.0.0.1 5000 1 127.0.0.1 5001
```

### Client
```bash
./client [server_ip] [port] [num_customers] [num_orders] [request_type]
```

**Request Types:**
- `1` - Write (robot order)
- `2` - Read (single customer record)
- `3` - Scan (print all customer records)

**Examples:**

Write 100 orders with 8 customers:
```bash
./client 127.0.0.1 5000 8 100 1
```

Read records for 5 customers:
```bash
./client 127.0.0.1 5000 5 10 2
```

Scan first 50 customer IDs:
```bash
./client 127.0.0.1 5000 1 50 3
```

## 🧪 Testing

### Basic Functionality Test
```bash
# Start 3 servers
./server 6000 0 2 1 127.0.0.1 6001 2 127.0.0.1 6002
./server 6001 1 2 0 127.0.0.1 6000 2 127.0.0.1 6002
./server 6002 2 2 0 127.0.0.1 6000 1 127.0.0.1 6001

# Write data to primary
./client 127.0.0.1 6000 5 10 1

# Verify replication on all servers
./client 127.0.0.1 6000 1 20 3
./client 127.0.0.1 6001 1 20 3
./client 127.0.0.1 6002 1 20 3
```

### Primary Failure Test
```bash
# Start writing
./client 127.0.0.1 6000 10 100 1 &

# Kill primary mid-execution
sleep 2
pkill -f "server 6000"

# Promote backup to new primary
./client 127.0.0.1 6001 5 10 1
```

### Backup Failure Test
```bash
# Start writing to primary
./client 127.0.0.1 6000 10 100 1 &

# Kill backup mid-execution
sleep 2
pkill -f "server 6001"

# Primary should continue with remaining backup
```

## 📊 Performance

Measured on Khoury Linux machines:

### Write Performance (8 customers, 99K orders)

| Servers | Mean Latency | Throughput    |
|---------|--------------|---------------|
| 1       | 95.8 μs      | 79,168 ops/s  |
| 2       | 155.7 μs     | 50,358 ops/s  |
| 3       | 231.6 μs     | 34,218 ops/s  |

**Observation**: Replication overhead increases latency by 142% and decreases throughput by 57% (1→3 servers).

### Read Performance (128 customers, 12K reads)

| Configuration | Throughput     |
|---------------|----------------|
| 1 Client      | 142,077 ops/s  |
| 2 Clients     | 144,639 ops/s  |

**Observation**: Concurrent reads from different servers show minimal scalability (+1.8%) due to lock contention.

## 🔍 Implementation Details

### Synchronization

- `admin_q_lock`: Protects engineer-to-admin request queue
- `log_lock`: Protects state machine log access
- `record_lock`: Protects customer_record map
- `admin_cv`: Condition variable for engineer-admin coordination

### Handshake Protocol

- 8-byte identification tag sent after connection
- `"CLIENT\0\0"` - Client connection
- `"PFA\0\0\0\0\0"` - Primary-to-backup replication connection

### Failure Detection

- Socket errors (`recv()/send()` returning ≤ 0) indicate failures
- Failed connections marked as `nullptr` and skipped
- Clients detect failures and exit gracefully

## 📁 Project Structure
```
.
├── ClientMain.cpp          # Client entry point
├── ClientSocket.{cpp,h}    # Client socket wrapper
├── ClientStub.{cpp,h}      # Client-side RPC stubs
├── ClientThread.{cpp,h}    # Customer thread implementation
├── ClientTimer.{cpp,h}     # Performance measurement
├── ServerMain.cpp          # Server entry point
├── ServerSocket.{cpp,h}    # Server socket wrapper
├── ServerStub.{cpp,h}      # Server-side RPC stubs
├── ServerThread.{cpp,h}    # Factory roles (Engineer/PFA/IFA)
├── FactoryStub.{cpp,h}     # Inter-server replication stubs
├── Socket.{cpp,h}          # Base socket class
├── Messages.{cpp,h}        # Message structures
├── MapOp.h                 # State machine operation
├── PeerInfo.h              # Peer configuration
├── Handshake.h             # Connection identification
└── Makefile                # Build configuration
```

## 🐛 Known Limitations

- **No automatic recovery**: Failed servers cannot automatically rejoin and catch up (bonus feature not implemented)
- **No connection retry**: Primary connects to peers once; failed connections require manual intervention
- **Limited read scalability**: Shared locks limit parallelization benefits

## 📝 Assignment Context

This was Assignment 3 for CS 6650 (Fall 2024) focusing on:
- State machine replication
- Primary-backup protocols
- Fault tolerance in distributed systems
- Performance analysis of replication overhead

**Grade Components:**
- State machine log and customer records (3 pts)
- Client functionality (3 pts)
- Replication without failures (3 pts)
- Replication with failures (3 pts)
- Report (3 pts)
- Bonus: Recovery/rejoin (2 pts) - Not implemented

## 📖 References

- [Assignment Specification](link-to-pdf-if-public)
- Course: CS 6650 - Building Scalable Distributed Systems
- Institution: Northeastern University

## 👤 Author

Mahip Parekh

## 📜 License

Academic project - all rights reserved.

---

**Note**: This project was developed as coursework. Please respect academic integrity policies if you're a current student.

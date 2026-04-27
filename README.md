                                       *** MiniDB – High Performance In-Memory Database Engine (C++)***

MiniDB is a lightweight in-memory key-value database engine built in C++.
It supports fast in-memory storage, write-ahead logging (WAL) for crash recovery, snapshot persistence, TTL-based key expiration, and multi-client access via TCP.

The system is inspired by the internal architecture of databases such as Redis, where data is stored in memory for extremely fast access while durability is maintained using disk persistence mechanisms.

***Key Features***
1) High-Performance In-Memory Storage
Custom hash table implementation
O(1) average lookup time
Optimized for fast reads and writes

2) Write-Ahead Logging (WAL)
All write operations are logged before execution
Ensures durability during crashes
Database state can be recovered from the WAL

3) Snapshot Persistence
Periodically stores the full dataset on disk
Enables fast database recovery

4) TTL (Time-To-Live) Support
Keys can expire automatically
Background cleaner thread removes expired entries

Example:
SET session123 user_data 5

5) LRU Eviction
When memory capacity is exceeded
The Least Recently Used key is evicted

6) Multi-Threaded Concurrency
Thread-safe operations
Multiple clients handled simultaneously

8) TCP Server Interface
Clients can connect remotely via TCP
Commands can be executed over network connections

9) Crash Recovery
Database recovery sequence:
Load snapshot
Replay WAL
Restore latest consistent state


***System Architecture***
                +----------------------+
                |      TCP Clients     |
                | (Telnet / Netcat)    |
                +----------+-----------+
                           |
                           |
                    TCP Socket Server
                           |
                           v
                +----------------------+
                |      TCP Server      |
                |  Command Processor   |
                +----------+-----------+
                           |
                           v
                +----------------------+
                |     Storage Engine   |
                |  Shard Router       |
                +----------+-----------+
                           |
        ------------------------------------------
        |                |                       |
        v                v                       v
+---------------+  +---------------+   +---------------+
|  Hash Table   |  |  Hash Table   |   |  Hash Table   |
|   (Shard 1)   |  |   (Shard 2)   |   |   (Shard N)   |
+---------------+  +---------------+   +---------------+
        |                |                       |
        ------------------------------------------
                           |
                           v
                +----------------------+
                |   Write Ahead Log    |
                |      (wal.log)       |
                +----------------------+
                           |
                           v
                +----------------------+
                |   Snapshot Manager   |
                |    (snapshot.db)     |
                +----------------------+

***Persistence Design***

MiniDB uses two persistence mechanisms.

1) Write Ahead Log (WAL)
All write operations are recorded before execution.
Example WAL entries:
SET A 100
SET B 200
DEL A

If the database crashes, WAL is replayed to restore the latest state

2) Snapshot
Snapshots store the entire database state.
File: snapshot.db

Recovery process:

Load Snapshot
      ↓
Replay WAL
      ↓
Database Ready

***TTL Expiration***
Keys can have expiration times.
Example:  SET temp_key 123 3

The key will automatically expire after 3 seconds.
Expired keys are removed by a background cleaner thread.


***TCP Server***
MiniDB exposes a TCP server for client communication.

Default port: localhost:8080

Clients can connect using:  telnet localhost 8080

Example session:
SET A 10
OK

GET A
10

***Build Instructions***

Clone the repository:   git clone https://github.com/yourusername/minidb.git
                        cd minidb

Build using CMake:  mkdir build
                    cd build

                    cmake ..
                    mingw32-make

Run the database:   ./MiniDB.exe

Run stress tests:   ./StressTest.exe

***Testing***

MiniDB was tested with the following scenarios.

1) Functional Tests
SET / GET / DEL operations
Snapshot persistence
TTL expiration

2) Crash Recovery Test
Database was forcefully terminated and restarted.
WAL replay restored the latest data successfully.

3) Concurrency Test
A multi-threaded stress test simulated concurrent operations to verify:
 Thread safety
 No race conditions
 Stable performance under load

***Future Improvements***

Potential enhancements include:

Distributed replication
Cluster sharding
Redis protocol compatibility
Performance benchmarking
Monitoring and metrics
Asynchronous replication


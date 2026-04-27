#include "storage_engine.h"
#include "tcp_server.h"

#include <iostream>

int main()
{
    std::cout << "Starting MiniDB TCP Server...\n";

    // 4 shards
    StorageEngine engine(4);

    // Start TCP server on port 8080
    TCPServer server(8080, engine);

    server.start();   // Blocking call

    return 0;
}
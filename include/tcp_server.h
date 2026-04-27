#pragma once

#include "storage_engine.h"
#include <winsock2.h>   // <-- add this
#include <string>

class TCPServer {
public:
    TCPServer(int port, StorageEngine& engine);
    void start();

private:
    SOCKET server_fd;   // <-- change from int
    int port;
    StorageEngine& engine;

    void handleClient(SOCKET clientSocket);   // <-- change from int
};
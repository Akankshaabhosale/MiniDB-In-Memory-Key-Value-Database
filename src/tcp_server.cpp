#include "tcp_server.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <sstream>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

TCPServer::TCPServer(int p, StorageEngine& eng)
    : port(p), engine(eng), server_fd(INVALID_SOCKET) {}

void TCPServer::start()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return;
    }

    std::cout << "TCP Server listening on port " << port << "\n";

    while (true)
    {
        sockaddr_in client{};
        int clientSize = sizeof(client);

        SOCKET clientSocket =
            accept(server_fd, (sockaddr*)&client, &clientSize);

        if (clientSocket == INVALID_SOCKET)
            continue;

        std::thread(&TCPServer::handleClient,
                    this,
                    clientSocket).detach();
    }
}

void TCPServer::handleClient(SOCKET clientSocket)
{
    char buffer[1024];
    std::string accumulated;

    while (true)
    {
        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        accumulated.append(buffer, bytes);

        size_t pos;
        while ((pos = accumulated.find('\n')) != std::string::npos)
        {
            std::string input = accumulated.substr(0, pos);
            accumulated.erase(0, pos + 1);

            std::istringstream iss(input);
            std::string cmd;
            iss >> cmd;

            std::string response;

            if (cmd == "SET")
            {
                std::string key, value;
                int ttl = 0;

                iss >> key >> value;
                if (iss >> ttl)
                    engine.set(key, value, ttl);
                else
                    engine.set(key, value);

                response = "OK\n";
            }
            else if (cmd == "GET")
            {
                std::string key, value;
                iss >> key;

                if (engine.get(key, value))
                    response = value + "\n";
                else
                    response = "Key not found\n";
            }
            else if (cmd == "DEL")
            {
                std::string key;
                iss >> key;

                engine.del(key);
                response = "Deleted\n";
            }
            else if (cmd == "SNAPSHOT")
            {
                engine.createSnapshot();
                response = "Snapshot saved\n";
            }
            else
            {
                response = "Invalid command\n";
            }

            send(clientSocket, response.c_str(), response.size(), 0);
        }
    }

    closesocket(clientSocket);
}
#pragma once

#include <string>
#include <vector>

#include "Location.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"

class ServerManager {
   public:
    static const size_t MAX_CLIENTS;
    static const size_t READ_BUFFER_SIZE;

    ServerManager();
    ServerManager(const std::vector<ServerConfig> &serverConfig);
    ServerManager(const ServerManager &other);
    ServerManager &operator=(const ServerManager &other);
    ~ServerManager();

    int initServer();
    std::vector<int> finishServer() const;
    int acceptConnection();

    int getPort() const;
    in_addr_t getHost() const;
    int getFd() const;
    int readFromClient(int clientSocket);
    bool isClient(int clientSocket) const;
    std::vector<Server>::const_iterator findServer(const std::string &host) const;

   private:
    Logger logger;

    int socketFd;

    int port;
    in_addr_t host;
    std::vector<Server> servers;
    std::vector<int> clientSockets;
    HttpRequest request;

    int removeClient(int clientSocket);
};

#pragma once

#include <string>
#include <vector>

#include "Location.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "Server.hpp"

class ServerManager {
   public:
    static const size_t MAX_CLIENTS;

    ServerManager();
    ServerManager(const std::vector<ServerConfig>& serverConfig);
    ServerManager(const ServerManager &other);
    ServerManager &operator=(const ServerManager &other);
    ~ServerManager();

    int initServer();
    int finishServer() const;
    int acceptConnection();

    int getPort() const;
    in_addr_t getHost() const;
    int getFd() const;

   private:
    Logger logger;

    int socketFd;

    int port;
    in_addr_t host;
    std::vector<Server> servers;
};

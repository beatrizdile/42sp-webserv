#pragma once

#include <string>
#include <vector>

#include "Client.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"

class ServerManager {
   public:
    static const size_t MAX_CLIENTS;
    static const size_t BUFFER_SIZE;

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
    int processClientRequest(int clientSocket, std::vector<pollfd> &fdsToAdd);
    int sendClientResponse(int clientSocket);
    bool isClient(int clientSocket) const;

   private:
    Logger logger;

    int socketFd;

    int port;
    in_addr_t host;
    std::vector<Server> servers;
    std::vector<Client> clients;
    HttpRequest request;
    HttpResponse response;

    int removeClient(int clientSocket);
    Client &getClient(int clientSocket);
};

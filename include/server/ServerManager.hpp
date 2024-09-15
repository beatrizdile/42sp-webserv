#pragma once

#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
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
    int handleClient(int clientSocket);
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
    HttpResponse response;

    int removeClient(int clientSocket);
    int matchUriAndResponseClient(int clientSocket);
    std::string processRequest(const t_config &config, const std::string &uri, const std::map<std::string, std::string> &headers);
    std::string processGetRequest(const t_config& config, const std::string &path, const std::string &uri);
    std::string processPostRequest(const t_config& config, const std::string &path, const std::string &uri, const std::map<std::string, std::string> &headers);
    std::string processDeleteRequest(const t_config& config, const std::string &path);
    std::string processCgi(const t_config& config, std::string path, const std::string& uri, const std::map<std::string, std::string>& headers);
};

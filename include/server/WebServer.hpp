#pragma once

#include <vector>

#include "Config.hpp"
#include "Logger.hpp"
#include "ServerManager.hpp"

class WebServer {
   public:
    static const size_t MAX_EVENTS;

    WebServer();
    WebServer(const Config &config);
    WebServer(const WebServer &other);
    WebServer &operator=(const WebServer &other);
    ~WebServer();

    void setupServers();
    void finishServers();
    void runServers();

   private:
    Logger logger;

    std::vector<struct pollfd> fds;
    std::vector<ServerManager> servers;

    static void verifyServers(std::vector<ServerConfig> serversConfig);
    void handleClient(int clientFd);
    std::vector<ServerManager>::iterator findServerFd(int fd);
    void addNewClient(int serverFd);
    void removeClient(int clientfd);
};

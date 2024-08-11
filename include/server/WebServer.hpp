#pragma once

#include <vector>

#include "Config.hpp"
#include "Logger.hpp"
#include "Server.hpp"

class WebServer {
   public:
    const static int MAX_EVENTS;

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
    std::vector<Server> servers;

    void verifyServers() const;
    void handleClient(int clientFd);
    std::vector<Server>::iterator findServerFd(int fd);
    void addNewClient(int serverFd);
    void removeClient(int clientfd);
};

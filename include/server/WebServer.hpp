#pragma once

#include <vector>

#include "Config.hpp"
#include "Logger.hpp"
#include "Server.hpp"

class WebServer {
   public:
    WebServer();
    WebServer(const Config &config);
    WebServer(const WebServer &other);
    WebServer &operator=(const WebServer &other);
    ~WebServer();

    void setupServers();
    void finishServers();

   private:
    Logger logger;

    std::vector<struct pollfd> fds;
    std::vector<Server> servers;
};

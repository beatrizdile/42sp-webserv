#pragma once

#include <string>
#include <vector>

#include "Location.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"

class Server {
   public:
    static const size_t MAX_CLIENTS = 100;

    Server();
    Server(const ServerConfig &serverConfig);
    Server(const Server &other);
    Server &operator=(const Server &other);
    ~Server();

    int initServer();
    int finishServer() const;
    int acceptConnection();

    int getPort() const;
    in_addr_t getHost() const;
    std::string getName() const;
    int getFd() const;

   private:
    Logger logger;

    int socketFd;

    int port;
    in_addr_t host;
    std::string name;
    std::string root;
    std::string index;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<Location> locations;
    std::vector<std::pair<size_t, std::string> > errorPages;
};

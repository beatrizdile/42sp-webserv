#pragma once

#include <string>
#include <vector>

#include "LocationConfig.hpp"
#include "Logger.hpp"

class ServerConfig {
   public:
    ServerConfig();
    ServerConfig(const ServerConfig &other);
    ServerConfig &operator=(const ServerConfig &other);
    ~ServerConfig();

    bool parseServer(const std::string &serverString);
    void printConfig();

   private:
    Logger logger;

    int port;
    std::string host;
    std::string name;
    std::string root;
    std::string index;
    std::vector<LocationConfig> locations;

    bool parseAttribute(const std::vector<std::string> &elems);
};

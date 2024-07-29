#pragma once

#include <string>
#include <vector>

#include "LocationConfig.hpp"

class ServerConfig {
   public:
    ServerConfig();
    ServerConfig(const ServerConfig &other);
    ServerConfig &operator=(const ServerConfig &other);
    ~ServerConfig();

   private:
    int port;
    std::string host;
    std::string name;
    std::vector<LocationConfig> locations;
};

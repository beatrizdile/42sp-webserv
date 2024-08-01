#pragma once

#include <vector>

#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "utils.h"

class Config {
   public:
    Config();
    Config(const Config &other);
    Config &operator=(const Config &other);
    ~Config();

    bool loadConfig(std::string configFilePath);
    void printConfig();

   private:
    Logger logger;
    std::vector<ServerConfig> servers;

    bool parseServers(const std::string &fileString);
};

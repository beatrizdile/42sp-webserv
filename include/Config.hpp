#pragma once

#include <stdlib.h>

#include <vector>

#include "Logger.hpp"
#include "ServerConfig.hpp"

class Config {
   public:
    Config();
    Config(const Config &other);
    Config &operator=(const Config &other);
    ~Config();

    bool load(std::string configFilePath);

   private:
    Logger logger;
    std::vector<ServerConfig> servers;

    bool getServers(const std::string &fileString);
    void removeUnecessarySpaces(std::string &fileString);
    bool verifySpaceBetweenBlocks(const std::string &fileString, size_t &start, size_t &end);
};

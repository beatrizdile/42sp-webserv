#include "Config.hpp"

#include <stdlib.h>

#include <fstream>
#include <iostream>

Config::Config() : logger(Logger("CONFIG")), servers(std::vector<ServerConfig>()) {}

Config::Config(const Config &other) {
    *this = other;
}

Config &Config::operator=(const Config &other) {
    if (this != &other) {
        logger = other.logger;
        servers = other.servers;
    }
    return (*this);
}

Config::~Config() {}

bool Config::loadConfig(std::string configFilePath) {
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open()) {
        logger.error() << "Error to open file: " << configFilePath << std::endl;
        return (false);
    }

    std::string line;
    std::string fileString;
    while (std::getline(configFile, line)) {
        if (line.empty()) continue;
        fileString += " " + line + " ";
    }

    configFile.close();
    removeUnecessarySpaces(fileString);
    return (parseServers(fileString));
}

bool Config::parseServers(const std::string &fileString) {
    size_t startBlock = 0;
    size_t lastEndBlock = 0;

    while ((startBlock = fileString.find("server", startBlock)) != std::string::npos) {
        if (!verifySpaceBetweenBlocks(fileString, lastEndBlock, startBlock)) {
            logger.error() << "Invalid character between server blocks '" << fileString[lastEndBlock] << "' at position " << lastEndBlock << std::endl;
            return (false);
        }

        size_t openBracePos = fileString.find('{', startBlock);
        if (openBracePos == std::string::npos) {
            logger.error() << "No opening brace found for service on config file" << std::endl;
            return (false);
        }

        startBlock += 6;
        if (!verifySpaceBetweenBlocks(fileString, startBlock, openBracePos)) {
            logger.error() << "Invalid character between server block and brace '" << fileString[startBlock] << "' at position " << startBlock << std::endl;
            return (false);
        }

        size_t endBlock = 0;
        if (!verifyBracesBalance(fileString, openBracePos, endBlock)) {
            logger.error() << "Brackets are not balanced in config file" << std::endl;
            return (false);
        }

        std::string serverString = fileString.substr(openBracePos + 1, endBlock - openBracePos - 2);
        trim(serverString);
        ServerConfig serverConfig;
        if (!serverConfig.parseServer(serverString)) {
            logger.error() << "Error parsing server block" << std::endl;
            return (false);
        }
        servers.push_back(serverConfig);

        lastEndBlock = endBlock;
        startBlock = endBlock;
    }

    startBlock = fileString.size();
    if (!verifySpaceBetweenBlocks(fileString, lastEndBlock, startBlock)) {
        logger.error() << "Invalid character in end of file '" << fileString[lastEndBlock] << "' at position " << lastEndBlock << std::endl;
        return (false);
    }

    if (servers.empty()) {
        logger.error() << "No server blocks found in config file" << std::endl;
        return (false);
    }

    return (true);
}

void Config::printConfig() {
    logger.info() << "Printing configuration ----------" << std::endl;
    for (size_t i = 0; i < servers.size(); ++i) {
        servers[i].printConfig();
    }
}

#include "Config.hpp"

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
    return *this;
}

Config::~Config() {}

bool Config::load(std::string configFilePath) {
    std::ifstream configFile(configFilePath.c_str());

    if (!configFile.is_open()) {
        logger.error() << "Error to open file: " << configFilePath << std::endl;
        return false;
    }

    std::string line;
    std::string fileString;
    while (std::getline(configFile, line)) {
        if (line.empty()) {
            continue;
        }

        fileString += " " + line + " ";
    }

    configFile.close();

    removeUnecessarySpaces(fileString);

    return getServers(fileString);
}

bool Config::getServers(const std::string &fileString) {
    size_t start = 0;
    size_t lastEnd = 0;
    std::vector<std::string> serverBlocks;

    while ((start = fileString.find("server", start)) != std::string::npos) {
        if (!verifySpaceBetweenBlocks(fileString, lastEnd, start)) {
            return false;
        }

        size_t openBracePos = fileString.find('{', start);
        if (openBracePos == std::string::npos) {
            logger.error() << "No opening brace found in config file" << std::endl;
            return false;
        }

        size_t openBraces = 1;
        size_t end = openBracePos + 1;

        while (openBraces > 0 && end < fileString.size()) {
            if (fileString[end] == '{') {
                ++openBraces;
            } else if (fileString[end] == '}') {
                --openBraces;
            }
            ++end;
        }

        if (openBraces != 0) {
            logger.error() << "Brackets are not balanced in config file" << std::endl;
            return false;
        }

        std::string server = fileString.substr(openBracePos + 1, end - openBracePos - 2);
        removeUnecessarySpaces(server);
        serverBlocks.push_back(server);
        lastEnd = end;
        start = end;
    }

    start = fileString.size();
    if (!verifySpaceBetweenBlocks(fileString, lastEnd, start)) {
        return false;
    }

    if (serverBlocks.empty()) {
        logger.error() << "No server blocks found in config file" << std::endl;
        return false;
    }

    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        logger.debug() << "Server block: " << serverBlocks[i] << std::endl;
    }

    return true;
}

void Config::removeUnecessarySpaces(std::string &fileString) {
    size_t pos = 0;

    while ((pos = fileString.find("\t", pos)) != std::string::npos) {
        fileString.replace(pos, 1, " ");
    }

    pos = 0;
    while ((pos = fileString.find("  ", pos)) != std::string::npos) {
        fileString.replace(pos, 2, " ");
    }

    if (fileString[0] == ' ') {
        fileString.erase(0, 1);
    }

    if (fileString[fileString.size() - 1] == ' ') {
        fileString.erase(fileString.size() - 1, 1);
    }
}

bool Config::verifySpaceBetweenBlocks(const std::string &fileString, size_t &start, size_t &end) {
    while (start < end) {
        if (fileString[start] != ' ' && fileString[start] != '\t') {
            logger.error() << "Invalid character found outside server block '" << fileString[start] << "' at position " << start << std::endl;
            return false;
        }
        ++start;
    }

    return true;
}

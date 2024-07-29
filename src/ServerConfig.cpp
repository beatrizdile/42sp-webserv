#include "ServerConfig.hpp"

#include <algorithm>

#include "utils.h"

ServerConfig::ServerConfig() : logger(Logger("SERVER_CONFIG")), port(-1), host(""), name(""), root(""), index(""), locations(std::vector<LocationConfig>()) {}

ServerConfig::ServerConfig(const ServerConfig &other) {
    *this = other;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &other) {
    if (this != &other) {
        port = other.port;
        host = other.host;
        name = other.name;
        root = other.root;
        index = other.index;
        locations = other.locations;
    }
    return (*this);
}

ServerConfig::~ServerConfig() {}

bool ServerConfig::parseServer(const std::string &serverString) {
    size_t startBlock = 0;
    size_t lastEndBlock = 0;

    while ((startBlock = serverString.find(";", startBlock)) != std::string::npos) {
        std::string line = serverString.substr(lastEndBlock, startBlock - lastEndBlock);
        std::vector<std::string> elems;

        removeUnecessarySpaces(line);
        split(line, ' ', elems);

        if (!parseAttribute(elems)) {
            return (false);
        }

        startBlock++;
        lastEndBlock = startBlock;
    }

    return (true);
}

bool ServerConfig::parseAttribute(const std::vector<std::string> &elems) {
    if (elems.size() == 0) {
        return (false);
    }

    if (elems[0] == "listen") {
        if (elems.size() != 2) {
            logger.error() << "Port attribute must have a value" << std::endl;
            return (false);
        }

        char *end;
        long value = std::strtol(elems[1].c_str(), &end, 10);

        if (*end != '\0' || value < 0 || value > 65535) {
            logger.error() << "Port attribute must be a number in valid range" << std::endl;
            return (false);
        }

        port = value;
    } else if (elems[0] == "host") {
        if (elems.size() != 2) {
            logger.error() << "Host attribute must have a value" << std::endl;
            return (false);
        }

        host = elems[1];
    } else if (elems[0] == "name") {
        if (elems.size() != 2) {
            logger.error() << "Name attribute must have a value" << std::endl;
            return (false);
        }

        name = elems[1];
    } else if (elems[0] == "root") {
        if (elems.size() != 2) {
            logger.error() << "Name attribute must have a value" << std::endl;
            return (false);
        }

        root = elems[1];
    } else if (elems[0] == "index") {
        if (elems.size() != 2) {
            logger.error() << "Name attribute must have a value" << std::endl;
            return (false);
        }

        index = elems[1];
    } else {
        logger.error() << "Unknown attribute: " << elems[0] << std::endl;
        return (false);
    }

    return (true);
}

void ServerConfig::printConfig() {
    logger.info() << "Server configuration ------------" << std::endl;
    logger.info() << "Port: " << port << std::endl;
    logger.info() << "Host: " << host << std::endl;
    logger.info() << "Name: " << name << std::endl;
    logger.info() << "Root: " << root << std::endl;
    logger.info() << "Index: " << index << std::endl;
}

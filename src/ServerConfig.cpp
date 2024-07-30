#include "ServerConfig.hpp"

#include <algorithm>

#include "utils.h"

std::string ServerConfig::LISTEN_KEY = "listen";
std::string ServerConfig::SERVER_NAME_KEY = "server_name";

ServerConfig::ServerConfig() : logger(Logger("SERVER_CONFIG")), port(-1), host(""), name(""), root(""), index(""), clientBodySize(LocationConfig::DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>()), locations(std::vector<LocationConfig>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

ServerConfig::ServerConfig(const ServerConfig& other) {
    *this = other;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
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

bool ServerConfig::processLocations(std::string& fileString) {
    size_t locationBlock = 0;

    while ((locationBlock = fileString.find("location", locationBlock)) != std::string::npos) {
        size_t startBlock = fileString.find("{", locationBlock);
        if (startBlock == std::string::npos) {
            logger.error() << "Location block must start with '{'" << std::endl;
            return false;
        }

        size_t locationPos = locationBlock + 8;
        std::string locationString = fileString.substr(locationPos, startBlock - locationPos);
        trim(locationString);
        if (locationString.empty() || locationString.find(" ") != std::string::npos) {
            logger.error() << "Location block must not contain spaces" << std::endl;
            return false;
        }

        size_t endBlock = fileString.find("}", startBlock);
        if (endBlock == std::string::npos) {
            logger.error() << "Location block must end with '}'" << std::endl;
            return false;
        }

        std::string locationBlockString = fileString.substr(startBlock + 1, endBlock - startBlock - 2);
        fileString.erase(locationBlock, endBlock - locationBlock + 1);
        trim(locationBlockString);

        LocationConfig locationConfig;
        if (!locationConfig.parseLocationBlock(locationBlockString, locationString)) {
            return false;
        }
        locations.push_back(locationConfig);

        locationBlock = 0;
    }

    return true;
}

bool ServerConfig::parseServer(const std::string& serverString) {
    size_t startBlock = 0;
    size_t lastEndBlock = 0;
    std::string server = serverString;

    if (!processLocations(server)) {
        return (false);
    }

    while ((startBlock = server.find(";", startBlock)) != std::string::npos) {
        std::string line = server.substr(lastEndBlock, startBlock - lastEndBlock);
        std::vector<std::string> elems;

        trim(line);
        split(line, ' ', elems);

        if (!parseAttribute(elems)) {
            return (false);
        }

        startBlock++;
        lastEndBlock = startBlock;
    }

    if (port == -1) {
        logger.error() << "Port attribute is required" << std::endl;
        return false;
    }

    if (methods.size() == 0) {
        methods.push_back(GET);
    }

    if (index.empty()) {
        index = "index.html";
    }

    return (true);
}
bool ServerConfig::processListen(const std::vector<std::string>& elems) {
    if (elems.size() != 2) {
        logger.error() << "Port attribute must have a value" << std::endl;
        return false;
    }

    std::vector<std::string> listens;
    std::string portString;
    split(elems[1], ':', listens);
    if (listens.size() > 2) {
        logger.error() << "Port attribute must be in the format 'host:port'" << std::endl;
        return false;
    } else if (listens.size() == 2) {
        host = listens[0];
        portString = listens[1];
    } else {
        portString = listens[0];
    }

    char* end;
    long value = std::strtol(portString.c_str(), &end, 10);

    if (*end != '\0' || value < 0 || value > 65535) {
        logger.error() << "Port attribute must be a number in valid range" << std::endl;
        return false;
    }

    port = value;
    return true;
}

bool ServerConfig::processName(const std::vector<std::string>& elems) {
    if (elems.size() != 2) {
        logger.error() << "Name attribute must have a value" << std::endl;
        return false;
    }

    name = elems[1];
    return true;
}

bool ServerConfig::processRoot(const std::vector<std::string>& elems) {
    if (elems.size() != 2) {
        logger.error() << "Root attribute must have a value" << std::endl;
        return false;
    }

    root = elems[1];
    return true;
}

bool ServerConfig::processIndex(const std::vector<std::string>& elems) {
    if (elems.size() != 2) {
        logger.error() << "Index attribute must have a value" << std::endl;
        return false;
    }

    index = elems[1];
    return true;
}

bool ServerConfig::processClientBodySize(const std::vector<std::string>& elems) {
    if (elems.size() != 2) {
        logger.error() << "Client body size attribute must have a value" << std::endl;
        return false;
    }

    char* end;
    long bodySize = std::strtol(elems[1].c_str(), &end, 10);
    if (*end != '\0' || bodySize < 0) {
        logger.error() << "Client body size attribute must be a number in valid range" << std::endl;
        return false;
    }

    clientBodySize = bodySize;
    return true;
}

bool ServerConfig::processMethod(const std::vector<std::string>& elems) {
    if (elems.size() < 2) {
        logger.error() << "Method attribute must have a value" << std::endl;
        return false;
    }

    for (size_t i = 1; i < elems.size(); i++) {
        Method method = getMethod(elems[i]);
        if (method == INVALID) {
            logger.error() << "Invalid method: " << elems[i] << std::endl;
            return false;
        }

        if (std::find(methods.begin(), methods.end(), method) == methods.end()) {
            methods.push_back(method);
        } else {
            logger.error() << "Method already exists: " << elems[i] << std::endl;
            return false;
        }
    }

    return true;
}

bool ServerConfig::processErrorPage(const std::vector<std::string>& elems) {
    if (elems.size() != 3) {
        logger.error() << "Error page attribute must have a value" << std::endl;
        return false;
    }

    char* end;
    long code = std::strtol(elems[1].c_str(), &end, 10);
    if (*end != '\0' || code < 100 || code > 599) {
        logger.error() << "Error code must be a number in valid range" << std::endl;
        return false;
    }

    errorPages.push_back(std::make_pair(code, elems[2]));
    return true;
}

bool ServerConfig::parseAttribute(const std::vector<std::string>& elems) {
    if (elems.size() == 0) {
        return false;
    }

    if (elems[0] == ServerConfig::LISTEN_KEY) {
        return processListen(elems);
    } else if (elems[0] == ServerConfig::SERVER_NAME_KEY) {
        return processName(elems);
    } else if (elems[0] == LocationConfig::ROOT_KEY) {
        return processRoot(elems);
    } else if (elems[0] == LocationConfig::INDEX_KEY) {
        return processIndex(elems);
    } else if (elems[0] == LocationConfig::CLIENT_BODY_SIZE_KEY) {
        return processClientBodySize(elems);
    } else if (elems[0] == LocationConfig::ALLOW_METHODS_KEY) {
        return processMethod(elems);
    } else if (elems[0] == LocationConfig::ERROR_PAGE_KEY) {
        return processErrorPage(elems);
    } else {
        logger.error() << "Unknown attribute: " << elems[0] << std::endl;
        return false;
    }
}

void ServerConfig::printConfig() {
    logger.info() << "Server configuration ------------" << std::endl;
    logger.info() << "Port: " << port << std::endl;
    logger.info() << "Host: " << host << std::endl;
    logger.info() << "Name: " << name << std::endl;
    logger.info() << "Root: " << root << std::endl;
    logger.info() << "Index: " << index << std::endl;
    logger.info() << "Client body size: " << clientBodySize << std::endl;

    std::string methodsString;
    for (size_t i = 0; i < methods.size(); i++) {
        methodsString += getMethodString(methods[i]) + " ";
    }
    logger.info() << "Methods: " << methodsString << std::endl;

    std::string errorPagesString;
    for (size_t i = 0; i < errorPages.size(); i++) {
        errorPagesString += errorPages[i].first + " " + errorPages[i].second + " ";
    }
    logger.info() << "Error pages: " << errorPagesString << std::endl;

    for (size_t i = 0; i < locations.size(); i++) {
        locations[i].printConfig();
    }
}

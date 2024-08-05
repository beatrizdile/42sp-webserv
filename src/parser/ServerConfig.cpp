#include "ServerConfig.hpp"

#include <algorithm>

#include "utils.h"

std::string ServerConfig::LISTEN_KEY = "listen";
std::string ServerConfig::SERVER_NAME_KEY = "server_name";
std::string ServerConfig::LOCATION_KEY = "location";

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
        clientBodySize = other.clientBodySize;
        methods = other.methods;
        errorPages = other.errorPages;
    }
    return (*this);
}

ServerConfig::~ServerConfig() {}

void ServerConfig::parseServer(const AstNode& node) {
    if (node.getIsLeaf()) {
        throw std::runtime_error("Server block is empty at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 0) {
        throw std::runtime_error("Server block has invalid values at line: " + numberToString(node.getKey().getLine()));
    }

    std::vector<AstNode*> children = node.getChildren();
    if (children.size() == 0) {
        throw std::runtime_error("Server block is empty at line: " + numberToString(node.getKey().getLine()));
    }

    for (size_t i = 0; i < children.size(); i++) {
        std::string attribute = children[i]->getKey().getValue();

        if (attribute == ServerConfig::LISTEN_KEY) {
            parseListen(*children[i]);
        } else if (attribute == ServerConfig::SERVER_NAME_KEY) {
            parseName(*children[i]);
        } else if (attribute == ServerConfig::LOCATION_KEY) {
            LocationConfig locationConfig;
            locationConfig.parseLocation(*children[i]);
            locations.push_back(locationConfig);
        } else if (attribute == LocationConfig::ROOT_KEY) {
            parseRoot(*children[i]);
        } else if (attribute == LocationConfig::INDEX_KEY) {
            parseIndex(*children[i]);
        } else if (attribute == LocationConfig::CLIENT_BODY_SIZE_KEY) {
            parseClientBodySize(*children[i]);
        } else if (attribute == LocationConfig::ALLOW_METHODS_KEY) {
            parseMethod(*children[i]);
        } else if (attribute == LocationConfig::ERROR_PAGE_KEY) {
            parseErrorPage(*children[i]);
        } else {
            throw std::runtime_error("Unknown attribute '" + attribute + "' in server block at line: " + numberToString(node.getKey().getLine()));
        }
    }
}

void ServerConfig::parseListen(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Listen attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Listen attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    std::string value = node.getValues().front().getValue();
    std::vector<std::string> parts;
    std::string portString;
    split(value, ':', parts);

    if (parts.size() > 2) {
        throw std::runtime_error("Port attribute must be in the format '<host>:<port> or <port>' at line: " + numberToString(node.getKey().getLine()));
    } else if (parts.size() == 2) {
        host = parts[0];
        portString = parts[1];
    } else {
        portString = parts[0];
    }

    char* end;
    long portNumber = std::strtol(portString.c_str(), &end, 10);
    if (*end != '\0' || portNumber < 0 || portNumber > 65535) {
        throw std::runtime_error("Port attribute must be a number in valid range at line: " + numberToString(node.getKey().getLine()));
    }

    port = portNumber;
}

void ServerConfig::parseName(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Server name attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Server name attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    name = node.getValues().front().getValue();
}

void ServerConfig::parseRoot(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Root attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Root attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    root = node.getValues().front().getValue();
}

void ServerConfig::parseIndex(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Index attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Index attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    index = node.getValues().front().getValue();
}

void ServerConfig::parseClientBodySize(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Client body size attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Client body size attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    std::string value = node.getValues().front().getValue();
    char* end;
    long size = std::strtol(value.c_str(), &end, 10);
    if (*end != '\0' || size < 0) {
        throw std::runtime_error("Client body size attribute must be a number at line: " + numberToString(node.getKey().getLine()));
    }

    clientBodySize = size;
}

void ServerConfig::parseMethod(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Method attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    std::vector<Token> elems = node.getValues();
    if (elems.size() < 1) {
        throw std::runtime_error("Method attribute must have at least one values at line: " + numberToString(node.getKey().getLine()));
    }

    for (size_t i = 1; i < elems.size(); i++) {
        Method method = getMethod(elems[i].getValue());
        if (method == INVALID) {
            throw std::runtime_error("Invalid method: '" + elems[i].getValue() + "' at line: " + numberToString(elems[i].getLine()));
        }

        if (std::find(methods.begin(), methods.end(), method) == methods.end()) {
            methods.push_back(method);
        } else {
            throw std::runtime_error("Method '" + elems[i].getValue() + "' already exists at line: " + numberToString(elems[i].getLine()));
        }
    }
}

void ServerConfig::parseErrorPage(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Error page attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    std::vector<Token> elems = node.getValues();
    if (elems.size() != 2) {
        throw std::runtime_error("Error page attribute must have two values at line: " + numberToString(node.getKey().getLine()));
    }

    char* end;
    long code = std::strtol(elems[0].getValue().c_str(), &end, 10);
    if (*end != '\0' || code < 100 || code > 599) {
        throw std::runtime_error("Error page code must be a number in valid range at line: " + numberToString(elems[0].getLine()));
    }

    errorPages.push_back(std::make_pair(code, elems[1].getValue()));
}

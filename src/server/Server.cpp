#include "Server.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

Server::Server() : logger(Logger("SERVER")), port(-1), host(INADDR_ANY), name(""), root(""), index(LocationConfig::DEFAULT_INDEX), clientBodySize(LocationConfig::DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>(GET)), locations(std::vector<Location>()), errorPages(std::vector<std::pair<size_t, std::string> >()), autoindex(false) {}

Server::Server(const ServerConfig &serverConfig) {
    logger = Logger("SERVER");
    port = serverConfig.getPort();
    host = serverConfig.getHost();
    name = serverConfig.getName();
    root = serverConfig.getRoot();
    index = serverConfig.getIndex();
    clientBodySize = serverConfig.getClientBodySize();
    methods = serverConfig.getMethods();
    if (methods.empty()) {
        methods.push_back(GET);
    }
    errorPages = serverConfig.getErrorPages();
    autoindex = serverConfig.getAutoindex();

    std::vector<LocationConfig> locationsConfig = serverConfig.getLocations();
    for (std::vector<LocationConfig>::iterator it = locationsConfig.begin(); it != locationsConfig.end(); ++it) {
        locations.push_back(Location(*it));
    }
}

Server::Server(const Server &other) {
    *this = other;
}

Server &Server::operator=(const Server &other) {
    if (this != &other) {
        logger = other.logger;
        port = other.port;
        host = other.host;
        name = other.name;
        root = other.root;
        index = other.index;
        clientBodySize = other.clientBodySize;
        methods = other.methods;
        locations = other.locations;
        errorPages = other.errorPages;
        autoindex = other.autoindex;
    }
    return (*this);
}

Server::~Server() {}

int Server::getPort() const {
    return (port);
}

in_addr_t Server::getHost() const {
    return (host);
}

std::vector<Location>::const_iterator Server::matchUri(std::string uri) const {
    // Exact Match
    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); it++) {
        if ((*it).getPath() == uri) {
            return (it);
        }
    }

    // Prefix Match
    std::vector<Location>::const_iterator longest = locations.end();
    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); it++) {
        if (uri.find((*it).getPath()) == 0) {
            if (longest == locations.end() || (*it).getPath() > (*longest).getPath()) {
                longest = it;
            }
        }
    }

    return (longest);
}

const std::string &Server::getName() const {
    return (name);
}

const std::string &Server::getRoot() const {
    return (root);
}

bool Server::getAutoindex() const {
    return (autoindex);
}

const std::vector<Location> &Server::getLocations() const {
    return (locations);
}

const std::string &Server::getIndex() const {
    return (index);
}

const std::vector<Method> &Server::getMethods() const {
    return (methods);
}

size_t Server::getClientBodySize() const {
    return (clientBodySize);
}

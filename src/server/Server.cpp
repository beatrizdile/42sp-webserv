#include "Server.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstring>

Server::Server() : logger(Logger("SERVER")), port(-1), host(INADDR_ANY), name(""), root(""), index(""), clientBodySize(LocationConfig::DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>()), locations(std::vector<Location>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

Server::Server(const ServerConfig& serverConfig) {
    logger = Logger("SERVER");
    port = serverConfig.getPort();
    host = serverConfig.getHost();
    name = serverConfig.getName();
    root = serverConfig.getRoot();
    index = serverConfig.getIndex();
    clientBodySize = serverConfig.getClientBodySize();
    methods = serverConfig.getMethods();
    errorPages = serverConfig.getErrorPages();

    std::vector<LocationConfig> locationsConfig = serverConfig.getLocations();
    for (size_t i = 0; i < locationsConfig.size(); i++) {
        locations.push_back(Location(locationsConfig[i]));
    }
}

Server::Server(const Server& other) {
    *this = other;
}

Server& Server::operator=(const Server& other) {
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
    }
    return (*this);
}

Server::~Server() {}

int Server::initServer() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        close(socketFd);
        std::string error_message = std::strerror(errno);
        std::string full_message = "setsockopt: " + error_message;
        throw std::runtime_error(full_message);
    }

    struct sockaddr_in server_addr;
    std::memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = host;
    server_addr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::string error_message = std::strerror(errno);
        std::string full_message = "bind: " + error_message;
        throw std::runtime_error(full_message);
    };

    return (socketFd);
}

int Server::finishServer() {
    close(socketFd);
    return (socketFd);
}

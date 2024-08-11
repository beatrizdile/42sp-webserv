#include "Server.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

Server::Server() : logger(Logger("SERVER")), socketFd(0), port(-1), host(INADDR_ANY), name(""), root(""), index(""), clientBodySize(LocationConfig::DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>()), locations(std::vector<Location>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

Server::Server(const ServerConfig& serverConfig) {
    logger = Logger("SERVER");
    socketFd = 0;
    port = serverConfig.getPort();
    host = serverConfig.getHost();
    name = serverConfig.getName();
    root = serverConfig.getRoot();
    index = serverConfig.getIndex();
    clientBodySize = serverConfig.getClientBodySize();
    methods = serverConfig.getMethods();
    errorPages = serverConfig.getErrorPages();

    std::vector<LocationConfig> locationsConfig = serverConfig.getLocations();
    for (std::vector<LocationConfig>::iterator it = locationsConfig.begin(); it != locationsConfig.end(); ++it) {
        locations.push_back(Location(*it));
    }
}

Server::Server(const Server& other) {
    *this = other;
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        logger = other.logger;
        socketFd = other.socketFd;
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
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        close(socketFd);
        throw createError("setsockopt");
    }

    struct sockaddr_in server_addr;
    std::memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = host;
    server_addr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw createError("bind");
    };

    if (listen(socketFd, MAX_CLIENTS) == -1) {
        throw createError("listen");
    }

    logger.info() << "Server " << name << " started on port " << port << std::endl;

    return (socketFd);
}

int Server::finishServer() const {
    if (socketFd != 0)
        close(socketFd);
    return (socketFd);
}

int Server::acceptConnection() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientFd = accept(socketFd, (struct sockaddr*)&clientAddress, &clientAddressLen);

    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags == -1) {
        close(clientFd);
        logger.perror("fcntl");
        return (-1);
    }
    if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(clientFd);
        logger.perror("fcntl");
        return (-1);
    }
    return (clientFd);
}

int Server::getPort() const {
    return (port);
}

in_addr_t Server::getHost() const {
    return (host);
}

std::string Server::getName() const {
    return (name);
}

int Server::getFd() const {
    return (socketFd);
}

#include "ServerManager.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

const size_t ServerManager::MAX_CLIENTS = 1000;

ServerManager::ServerManager() : logger(Logger("SERVER_MANAGER")), socketFd(0), port(-1), host(INADDR_ANY), servers(std::vector<Server>()) {}

ServerManager::ServerManager(const std::vector<ServerConfig>& serverConfig) {
    logger = Logger("SERVER_MANAGER");
    socketFd = 0;
    port = serverConfig.front().getPort();
    host = serverConfig.front().getHost();
    for (std::vector<ServerConfig>::const_iterator it = serverConfig.begin(); it != serverConfig.end(); ++it) {
        servers.push_back(Server(*it));
    }
}

ServerManager::ServerManager(const ServerManager& other) {
    *this = other;
}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    if (this != &other) {
        logger = other.logger;
        socketFd = other.socketFd;
        port = other.port;
        host = other.host;
        servers = other.servers;
    }
    return (*this);
}

ServerManager::~ServerManager() {}

int ServerManager::initServer() {
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

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &host, ipStr, INET_ADDRSTRLEN);
    logger.info() << "ServerManager " << ipStr << " started on port " << port << std::endl;

    return (socketFd);
}

int ServerManager::finishServer() const {
    if (socketFd != 0)
        close(socketFd);
    return (socketFd);
}

int ServerManager::acceptConnection() {
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

int ServerManager::getPort() const {
    return (port);
}

in_addr_t ServerManager::getHost() const {
    return (host);
}

int ServerManager::getFd() const {
    return (socketFd);
}

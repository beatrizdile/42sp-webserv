#include "ServerManager.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>

const size_t ServerManager::MAX_CLIENTS = 1000;
const size_t ServerManager::BUFFER_SIZE = 1024;

ServerManager::ServerManager() : logger(Logger("SERVER_MANAGER")), socketFd(0), port(-1), host(INADDR_ANY), servers(std::vector<Server>()), clients(std::vector<Client>()), request(HttpRequest()), response(HttpResponse()) {}

ServerManager::ServerManager(const std::vector<ServerConfig>& serverConfig) {
    logger = Logger("SERVER_MANAGER");
    socketFd = 0;
    port = serverConfig.front().getPort();
    host = serverConfig.front().getHost();
    clients = std::vector<Client>();
    request = HttpRequest();
    response = HttpResponse();

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
        clients = other.clients;
        request = other.request;
        response = other.response;
    }
    return (*this);
}

ServerManager::~ServerManager() {}

int ServerManager::initServer() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

#ifndef __APPLE__
    int optval = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        close(socketFd);
        throw createError("setsockopt");
    }
#endif

    struct sockaddr_in server_addr;
    std::memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = host;
    server_addr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw createError("bind");
    }

    if (listen(socketFd, MAX_CLIENTS) == -1) {
        throw createError("listen");
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &host, ipStr, INET_ADDRSTRLEN);
    logger.info() << "ServerManager " << ipStr << " started on port " << port << std::endl;

    return (socketFd);
}

void ServerManager::verifyClientsCgiTimeout(std::vector<int>& fdsToRemove) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        it->verifyCgiTimeout(fdsToRemove);
    }
}

std::vector<int> ServerManager::finishServer() const {
    if (socketFd != 0)
        close(socketFd);

    std::vector<int> allFds;
    for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->getFd());
        allFds.push_back(it->getFd());
    }

    allFds.push_back(socketFd);
    return (allFds);
}

int ServerManager::acceptConnection() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientFd = accept(socketFd, (struct sockaddr*)&clientAddress, &clientAddressLen);
    if (clientFd == -1) {
        logger.perror("accept");
        return (-1);
    }

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

    clients.push_back(clientFd);
    return (clientFd);
}

int ServerManager::processClientRequest(int clientSocket, std::vector<pollfd>& fdsToAdd) {
    Client& client = getClient(clientSocket);
    int fd = client.processSendedData(clientSocket, servers, fdsToAdd);
    if (fd != 0) {
        return (fd == client.getFd() ? removeClient(fd) : fd);
    }
    return (0);
}

int ServerManager::processHandUp(int clientSocket) {
    Client& client = getClient(clientSocket);
    client.readCgiResponse();
    return (clientSocket);
}

int ServerManager::sendClientResponse(int clientSocket) {
    Client& client = getClient(clientSocket);
    int fd = client.sendResponse(clientSocket);
    if (fd != 0) {
        return (fd == client.getFd() ? removeClient(fd) : fd);
    }
    return (0);
}

int ServerManager::removeClient(int clientSocket) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((*it).getFd() == clientSocket) {
            it->closeAll();
            clients.erase(it);
            break;
        }
    }
    return (clientSocket);
}


bool ServerManager::isPipeOutClient(int clientSocket) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((*it).getPipeOut() == clientSocket) {
            return (true);
        }
    }
    return (false);
}

Client& ServerManager::getClient(int clientSocket) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((*it).isFdValid(clientSocket)) {
            return (*it);
        }
    }
    throw createError("Client not found");
}

bool ServerManager::isClient(int clientSocket) const {
    for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->isFdValid(clientSocket)) {
            return (true);
        }
    }
    return (false);
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

#include "WebServer.hpp"

#include <poll.h>

#include <algorithm>

WebServer::WebServer() : logger(Logger("SERVER_MANAGER")), servers(std::vector<Server>()) {}

WebServer::WebServer(const Config& config) {
    logger = Logger("SERVER_MANAGER");

    std::vector<ServerConfig> serversConfig = config.getServers();
    for (size_t i = 0; i < serversConfig.size(); i++) {
        servers.push_back(Server(serversConfig[i]));
    }
}

WebServer::WebServer(const WebServer& other) {
    *this = other;
}

WebServer& WebServer::operator=(const WebServer& other) {
    if (this != &other) {
        logger = other.logger;
        servers = other.servers;
    }
    return (*this);
}

WebServer::~WebServer() {
    finishServers();
}

void WebServer::setupServers() {
    for (size_t i = 0; i < servers.size(); i++) {
        int socketFd = servers[i].initServer();
        struct pollfd fd;
        fd.fd = socketFd;
        fd.events = POLLIN;
        fd.revents = 0;
        fds.push_back(fd);
    }
}

void WebServer::finishServers() {
    for (size_t i = 0; i < servers.size(); i++) {
        int socketFd = servers[i].finishServer();

        for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
            if (it->fd == socketFd) {
                fds.erase(it);
                break;
            }
        }
    }
}
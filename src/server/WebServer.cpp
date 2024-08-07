#include "WebServer.hpp"

#include <poll.h>

#include <algorithm>

WebServer::WebServer() : logger(Logger("SERVER_MANAGER")), servers(std::vector<Server>()) {}

WebServer::WebServer(const Config& config) {
    logger = Logger("SERVER_MANAGER");

    std::vector<ServerConfig> serversConfig = config.getServers();
    for (std::vector<ServerConfig>::iterator it = serversConfig.begin(); it != serversConfig.end(); ++it) {
        servers.push_back(Server(*it));
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
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        int socketFd = (*it).initServer();
        struct pollfd fd;
        fd.fd = socketFd;
        fd.events = POLLIN;
        fd.revents = 0;
        fds.push_back(fd);
    }
}

void WebServer::finishServers() {
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        int socketFd = (*it).finishServer();

        for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
            if (it->fd == socketFd) {
                fds.erase(it);
                break;
            }
        }
    }
}
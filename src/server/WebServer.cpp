#include "WebServer.hpp"

#include <poll.h>
#include <unistd.h>

#include <algorithm>

const int WebServer::MAX_EVENTS = 1000;

WebServer::WebServer() : logger(Logger("SERVER_MANAGER")), fds(std::vector<struct pollfd>()), servers(std::vector<Server>()) {
    fds.reserve(MAX_EVENTS);
}

WebServer::WebServer(const Config& config) {
    fds.reserve(MAX_EVENTS);
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
        fds = other.fds;
    }
    fds.reserve(MAX_EVENTS);
    return (*this);
}

WebServer::~WebServer() {
    finishServers();
}

void WebServer::setupServers() {
    verifyServers();

    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        int socketFd = (*it).initServer();
        struct pollfd fd;
        fd.fd = socketFd;
        fd.events = POLLIN;
        fd.revents = 0;
        fds.push_back(fd);
    }
}

void WebServer::handleClient(int clientFd) {
    char buffer[1024];
    int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        send(clientFd, buffer, bytesRead, 0);
    } else if (bytesRead == 0) {
        logger.info() << "Client disconnected" << std::endl;
        removeClient(clientFd);
    } else {
        logger.perror("recv");
    }
}

void WebServer::runServers() {
    std::vector<int> fdsToRemove;
    std::vector<int> fdsToAdd;

    while (true) {
        int affected = 0;
        int currentAffected = 0;
        if ((affected = poll(fds.data(), fds.size(), -1)) < 0) {
            throw createError("poll");
        }

        for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end() && currentAffected < affected; ++it) {
            if ((*it).revents & POLLIN) {
                currentAffected++;
                std::vector<Server>::iterator server = findServerFd((*it).fd);

                if (server != servers.end()) {
                    logger.info() << "New connection on fd " << (*it).fd << std::endl;
                    int clientFd = (*server).acceptConnection();
                    if (clientFd < 0) {
                        logger.perror("accept");
                        continue;
                    }
                    fdsToAdd.push_back(clientFd);
                } else {
                    handleClient((*it).fd);
                }
            } else if ((*it).revents & POLLERR) {
                currentAffected++;
                logger.error() << "Error on fd " << (*it).fd << std::endl;
                fdsToRemove.push_back((*it).fd);
            } else if ((*it).revents & POLLRDHUP) {
                currentAffected++;
                logger.info() << "Client disconnected on fd " << (*it).fd << std::endl;
                fdsToRemove.push_back((*it).fd);
            } else if ((*it).revents != 0) {
                currentAffected++;
                logger.info() << "Unknown event on fd " << (*it).fd << " events: " << (*it).revents << std::endl;
            }
        }

        for (std::vector<int>::iterator it = fdsToRemove.begin(); it != fdsToRemove.end(); ++it) {
            removeClient(*it);
        }
        fdsToRemove.clear();

        for (std::vector<int>::iterator it = fdsToAdd.begin(); it != fdsToAdd.end(); ++it) {
            addNewClient(*it);
        }
        fdsToAdd.clear();
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

void WebServer::verifyServers() const {
    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        for (std::vector<Server>::const_iterator it2 = it + 1; it2 != servers.end(); ++it2) {
            if ((*it).getPort() == (*it2).getPort() && (*it).getHost() == (*it2).getHost() && (*it).getName() == (*it2).getName()) {
                throw std::runtime_error("Server with port " + numberToString((*it).getPort()) + " host " + numberToString((*it).getHost()) + " and name \"" + (*it).getName() + "\" already exists");
            }
        }
    }
}

std::vector<Server>::iterator WebServer::findServerFd(int fd) {
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->getFd() == fd) {
            return (it);
        }
    }
    return (servers.end());
}

void WebServer::removeClient(int clientfd) {
    for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
        if ((*it).fd == clientfd) {
            fds.erase(it);
            close(clientfd);
            break;
        }
    }
}

void WebServer::addNewClient(int clientFd) {
    pollfd clientPollfd;
    clientPollfd.fd = clientFd;
    clientPollfd.events = POLLIN | POLLRDHUP | POLLERR;
    clientPollfd.revents = 0;
    fds.push_back(clientPollfd);
}

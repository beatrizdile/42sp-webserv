#include "WebServer.hpp"

#include <poll.h>
#include <unistd.h>

#include <algorithm>

const size_t WebServer::MAX_EVENTS = 1000;

WebServer::WebServer() : logger(Logger("SERVER_MANAGER")), fds(std::vector<struct pollfd>()), servers(std::vector<ServerManager>()) {
    fds.reserve(MAX_EVENTS);
}

WebServer::WebServer(const Config& config) {
    fds.reserve(MAX_EVENTS);
    logger = Logger("SERVER_MANAGER");

    std::vector<ServerConfig> serversConfig = config.getServers();
    verifyServers(serversConfig);

    std::vector<ServerConfig> serversWithSameHostPort;
    while (!serversConfig.empty()) {
        serversWithSameHostPort.clear();
        ServerConfig serverConfig = serversConfig.front();
        serversWithSameHostPort.push_back(serverConfig);
        serversConfig.erase(serversConfig.begin());

        for (std::vector<ServerConfig>::iterator it = serversConfig.begin(); it != serversConfig.end();) {
            if (serverConfig.getHost() == it->getHost() && serverConfig.getPort() == it->getPort()) {
                serversWithSameHostPort.push_back(*it);
                it = serversConfig.erase(it);
            } else {
                ++it;
            }
        }

        servers.push_back(ServerManager(serversWithSameHostPort));
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
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
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
                std::vector<ServerManager>::iterator server = findServerFd((*it).fd);

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
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
        int socketFd = (*it).finishServer();

        for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
            if (it->fd == socketFd) {
                fds.erase(it);
                break;
            }
        }
    }
}

void WebServer::verifyServers(std::vector<ServerConfig> serversConfig) {
    for (std::vector<ServerConfig>::const_iterator it = serversConfig.begin(); it != serversConfig.end(); ++it) {
        for (std::vector<ServerConfig>::const_iterator it2 = it + 1; it2 != serversConfig.end(); ++it2) {
            if ((*it).getPort() == (*it2).getPort() && (*it).getHost() == (*it2).getHost() && (*it).getName() == (*it2).getName()) {
                

                char ipStr[INET_ADDRSTRLEN];
                in_addr_t host = (*it).getHost();
                inet_ntop(AF_INET, &host, ipStr, INET_ADDRSTRLEN);
                throw std::runtime_error("Server with port " + numberToString((*it).getPort()) + " host " + ipStr + " and name \"" + (*it).getName() + "\" already exists");
            }
        }
    }
}

std::vector<ServerManager>::iterator WebServer::findServerFd(int fd) {
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
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

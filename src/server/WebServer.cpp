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
    verifyDuplicatedServers(serversConfig);

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

void WebServer::runServers() {
    std::vector<int> fdsToRemove;
    std::vector<pollfd> fdsToAdd;

    while (true) {
        try {
            int affected = 0;
            int currentAffected = 0;
            if ((affected = poll(fds.data(), fds.size(), -1)) < 0) {
                throw createError("poll");
            }

            for (std::vector<struct pollfd>::iterator fd = fds.begin(); fd != fds.end() && currentAffected < affected; ++fd) {
                if ((*fd).revents == 0) {
                    continue;
                }

                currentAffected++;
                if ((*fd).revents & POLLIN) {
                    std::vector<ServerManager>::iterator server = findServerFd((*fd).fd);

                    if (server != servers.end()) {
                        logger.info() << "New connection on fd " << (*fd).fd << std::endl;
                        int clientFd = (*server).acceptConnection();
                        if (clientFd < 0) {
                            logger.perror("accept");
                            continue;
                        }
                        struct pollfd pfd;
                        pfd.fd = clientFd;
                        pfd.events = POLLIN | POLLOUT | POLLNVAL | POLLHUP | POLLERR;
                        pfd.revents = 0;
                        fdsToAdd.push_back(pfd);
                    } else {
                        std::vector<ServerManager>::iterator it = findServerClientFd((*fd).fd);
                        if ((*it).processClientRequest((*fd).fd, fdsToAdd) != 0) {
                            fdsToRemove.push_back((*fd).fd);
                        }
                    }
                } else if ((*fd).revents & POLLOUT) {
                    std::vector<ServerManager>::iterator it = findServerClientFd((*fd).fd);
                    if ((*it).sendClientResponse((*fd).fd) != 0) {
                        fdsToRemove.push_back((*fd).fd);
                    }
                } else if ((*fd).revents & POLLNVAL) {
                    logger.error() << "Invalid request on fd " << (*fd).fd << std::endl;
                    fdsToRemove.push_back((*fd).fd);
                } else if ((*fd).revents & POLLERR) {
                    logger.error() << "Error on fd " << (*fd).fd << std::endl;
                    fdsToRemove.push_back((*fd).fd);
                } else if ((*fd).revents & POLLHUP) {
                    std::vector<ServerManager>::iterator it = findServerClientPipeOutput((*fd).fd);
                    (*it).processHandUp((*fd).fd);
                    fdsToRemove.push_back((*fd).fd);
                    logger.info() << "Client disconnected on fd " << (*fd).fd << std::endl;
                } else {
                    logger.info() << "Unknown event on fd " << (*fd).fd << " events: " << (*fd).revents << std::endl;
                }
            }

            for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
                (*it).verifyClientsCgiTimeout(fdsToRemove);
            }

            for (std::vector<int>::iterator it = fdsToRemove.begin(); it != fdsToRemove.end(); ++it) {
                removeClient(*it);
            }
            fdsToRemove.clear();

            for (std::vector<pollfd>::iterator it = fdsToAdd.begin(); it != fdsToAdd.end(); ++it) {
                fds.push_back(*it);
            }
            fdsToAdd.clear();
        } catch (std::exception& e) {
            logger.error() << "Error: " << e.what() << std::endl;
        }
    }
}

std::vector<ServerManager>::iterator WebServer::findServerClientFd(int clientFd) {
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->isClient(clientFd)) {
            return (it);
        }
    }

    throw std::runtime_error("Client with fd " + numberToString(clientFd) + " not found");
}

std::vector<ServerManager>::iterator WebServer::findServerClientPipeOutput(int clientFd) {
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->isPipeOutClient(clientFd)) {
            return (it);
        }
    }

    throw std::runtime_error("Client with fd " + numberToString(clientFd) + " not found");
}

void WebServer::finishServers() {
    for (std::vector<ServerManager>::iterator it = servers.begin(); it != servers.end(); ++it) {
        std::vector<int> allSocketFd = (*it).finishServer();

        for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
            if (std::find(allSocketFd.begin(), allSocketFd.end(), (*it).fd) != allSocketFd.end()) {
                fds.erase(it);
            }
        }
    }
}

void WebServer::verifyDuplicatedServers(std::vector<ServerConfig> serversConfig) {
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

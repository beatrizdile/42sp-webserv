#pragma once

#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "config.hpp"

class Server {
   public:
    Server();
    Server(const ServerConfig &serverConfig);
    Server(const Server &other);
    Server &operator=(const Server &other);
    ~Server();

    int getPort() const;
    in_addr_t getHost() const;
    const std::string &getName() const;
    const std::string &getRoot() const;
    bool getAutoindex() const;
    const std::string &getIndex() const;
    const std::vector<Location> &getLocations() const;
    const std::vector<Method> &getMethods() const;
    size_t getClientBodySize() const;
    std::vector<Location>::const_iterator matchUri(std::string uri) const;
    const t_config getConfig() const;

   private:
    Logger logger;

    int port;
    in_addr_t host;
    std::string name;
    std::string root;
    std::string index;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<Location> locations;
    std::vector<std::pair<size_t, std::string> > errorPages;
    bool autoindex;
};

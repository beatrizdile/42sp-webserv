#pragma once

#include <arpa/inet.h>

#include <string>
#include <vector>

#include "AstNode.hpp"
#include "LocationConfig.hpp"
#include "Logger.hpp"

class ServerConfig {
   public:
    static std::string LISTEN_KEY;
    static std::string SERVER_NAME_KEY;
    static std::string LOCATION_KEY;

    ServerConfig();
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);
    ~ServerConfig();

    void parseServer(const AstNode& node);

    int getPort() const;
    in_addr_t getHost() const;
    std::string getName() const;
    std::string getRoot() const;
    std::string getIndex() const;
    size_t getClientBodySize() const;
    std::vector<Method> getMethods() const;
    std::vector<LocationConfig> getLocations() const;
    std::vector<std::pair<size_t, std::string> > getErrorPages() const;

   private:
    Logger logger;

    int port;
    in_addr_t host;
    std::string name;
    std::string root;
    std::string index;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<LocationConfig> locations;
    std::vector<std::pair<size_t, std::string> > errorPages;

    void parseListen(const AstNode& node);
    void parseName(const AstNode& node);
    void parseRoot(const AstNode& node);
    void parseIndex(const AstNode& node);
    void parseClientBodySize(const AstNode& node);
    void parseMethod(const AstNode& node);
    void parseErrorPage(const AstNode& node);
};

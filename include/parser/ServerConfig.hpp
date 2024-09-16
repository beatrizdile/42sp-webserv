#pragma once

#include <arpa/inet.h>

#include <string>
#include <vector>

#include "AstNode.hpp"
#include "LocationConfig.hpp"
#include "Logger.hpp"

class ServerConfig {
   public:
    static const std::string LISTEN_KEY;
    static const std::string SERVER_NAME_KEY;
    static const std::string LOCATION_KEY;

    ServerConfig();
    ServerConfig(const ServerConfig& other);
    ServerConfig& operator=(const ServerConfig& other);
    ~ServerConfig();

    void parseServer(const AstNode& node);

    int getPort() const;
    in_addr_t getHost() const;
    const std::string& getName() const;
    const std::string& getRoot() const;
    const std::string& getIndex() const;
    size_t getClientBodySize() const;
    const std::vector<Method>& getMethods() const;
    const std::vector<LocationConfig>& getLocations() const;
    const std::vector<std::pair<size_t, std::string> >& getErrorPages() const;
    bool getAutoindex() const;

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
    bool autoindex;

    void verifyDuplicatedLocations() const;
    void validMinimumConfig() const;

    void parseListen(const AstNode& node);
    void parseName(const AstNode& node);
    void parseRoot(const AstNode& node);
    void parseIndex(const AstNode& node);
    void parseClientBodySize(const AstNode& node);
    void parseMethod(const AstNode& node);
    void parseErrorPage(const AstNode& node);
    void parseAutoindex(const AstNode& node);
};

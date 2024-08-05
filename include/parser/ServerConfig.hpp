#pragma once

#include <string>
#include <vector>

#include "LocationConfig.hpp"
#include "AstNode.hpp"
#include "Logger.hpp"

typedef bool (*ProcessorFunc)(const std::vector<std::string> &);

class ServerConfig {
   public:
    static std::string LISTEN_KEY;
    static std::string SERVER_NAME_KEY;
    static std::string LOCATION_KEY;

    ServerConfig();
    ServerConfig(const ServerConfig &other);
    ServerConfig &operator=(const ServerConfig &other);
    ~ServerConfig();

    void parseServer(const AstNode& node);

   private:
    Logger logger;

    int port;
    std::string host;
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

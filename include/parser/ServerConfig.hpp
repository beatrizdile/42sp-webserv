#pragma once

#include <string>
#include <vector>

#include "LocationConfig.hpp"
#include "Logger.hpp"

typedef bool (*ProcessorFunc)(const std::vector<std::string> &);

class ServerConfig {
   public:
    static std::string LISTEN_KEY;
    static std::string SERVER_NAME_KEY;

    ServerConfig();
    ServerConfig(const ServerConfig &other);
    ServerConfig &operator=(const ServerConfig &other);
    ~ServerConfig();

    bool parseServer(const std::string &serverString);
    void printConfig();

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

    bool processLocations(std::string &fileString);
    bool parseAttribute(const std::vector<std::string> &elems);

    bool processListen(const std::vector<std::string> &elems);
    bool processName(const std::vector<std::string> &elems);
    bool processRoot(const std::vector<std::string> &elems);
    bool processIndex(const std::vector<std::string> &elems);
    bool processClientBodySize(const std::vector<std::string> &elems);
    bool processMethod(const std::vector<std::string> &elems);
    bool processErrorPage(const std::vector<std::string> &elems);
};

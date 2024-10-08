#pragma once

#include <string>
#include <vector>

#include "Configurations.hpp"
#include "LocationConfig.hpp"
#include "Logger.hpp"

class Location {
   public:
    Location();
    Location(const LocationConfig &locationConfig, const std::string &serverRoot);
    Location(const Location &other);
    Location &operator=(const Location &other);
    ~Location();

    const std::string &getPath() const;
    const std::string &getRoot() const;
    size_t getClientBodySize() const;
    const std::string &getIndex() const;
    const std::vector<Method> &getMethods() const;
    bool getAutoindex() const;
    const Configurations &getConfig() const;

   private:
    Logger logger;

    std::string path;
    std::string root;
    std::string index;
    std::string redirect;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<std::pair<size_t, std::string> > errorPages;
    bool autoindex;
    std::map<std::string, std::string> cgiPaths;
    Configurations config;
};

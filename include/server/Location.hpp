#pragma once

#include <string>
#include <vector>

#include "LocationConfig.hpp"
#include "Logger.hpp"

class Location {
   public:
    Location();
    Location(const LocationConfig &locationConfig);
    Location(const Location &other);
    Location &operator=(const Location &other);
    ~Location();

    std::string getPath() const;

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
};

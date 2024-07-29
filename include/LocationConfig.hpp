#pragma once

#include <string>
#include <vector>

#include "Method.hpp"

class LocationConfig {
   public:
    LocationConfig();
    LocationConfig(const LocationConfig &other);
    LocationConfig &operator=(const LocationConfig &other);
    ~LocationConfig();

   private:
    std::string path;
    std::string root;
    std::string index;
    std::string redirect;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<std::pair<size_t, std::string> > errorPages;
};

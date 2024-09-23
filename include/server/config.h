#pragma once

#include <map>
#include <string>
#include <vector>

#include "Method.hpp"

typedef struct s_config {
    bool isAutoindex;
    size_t clientBodySize;
    const std::string redirect;
    const std::string &root;
    const std::string &index;
    const std::vector<Method> &methods;
    const std::vector<std::pair<size_t, std::string> > &errorPages;
    const std::map<std::string, std::string> cgiPaths;
} t_config;

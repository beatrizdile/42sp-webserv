#include "LocationConfig.hpp"

LocationConfig::LocationConfig() : path(""), root(""), index(""), redirect(""), clientBodySize(-1), methods(std::vector<Method>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

LocationConfig::LocationConfig(const LocationConfig &other) {
    *this = other;
}

LocationConfig &LocationConfig::operator=(const LocationConfig &other) {
    if (this != &other) {
        path = other.path;
        root = other.root;
        index = other.index;
        redirect = other.redirect;
        clientBodySize = other.clientBodySize;
        methods = other.methods;
        errorPages = other.errorPages;
    }
    return (*this);
}

LocationConfig::~LocationConfig() {}

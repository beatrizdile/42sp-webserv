#include "Location.hpp"

Location::Location() : logger(Logger("LOCATION")), path(""), root(""), index(LocationConfig::DEFAULT_INDEX), redirect(""), clientBodySize(0), methods(std::vector<Method>()), errorPages(std::vector<std::pair<size_t, std::string> >()), autoindex(false), cgiPaths() {}

Location::Location(const LocationConfig& locationConfig, const std::string& serverRoot) {
    logger = Logger("LOCATION");
    path = locationConfig.getPath();
    root = locationConfig.getRoot();
    if (root.empty()) {
        root = serverRoot;
    }
    index = locationConfig.getIndex();
    redirect = locationConfig.getRedirect();
    clientBodySize = locationConfig.getClientBodySize();
    methods = locationConfig.getMethods();
    if (methods.empty()) {
        methods.push_back(GET);
    }
    errorPages = locationConfig.getErrorPages();
    autoindex = locationConfig.getAutoindex();
    cgiPaths = locationConfig.getCgiPaths();
}

Location::Location(const Location& other) {
    *this = other;
}

Location& Location::operator=(const Location& other) {
    if (this != &other) {
        logger = other.logger;
        path = other.path;
        root = other.root;
        index = other.index;
        redirect = other.redirect;
        clientBodySize = other.clientBodySize;
        methods = other.methods;
        errorPages = other.errorPages;
        autoindex = other.autoindex;
        cgiPaths = other.cgiPaths;
    }
    return (*this);
}

Location::~Location() {}

const std::string& Location::getPath() const {
    return (path);
}

const std::string& Location::getRoot() const {
    return (root);
}

bool Location::getAutoindex() const {
    return (autoindex);
}

const std::string& Location::getIndex() const {
    return (index);
}

const std::vector<Method>& Location::getMethods() const {
    return (methods);
}

size_t Location::getClientBodySize() const {
    return (clientBodySize);
}

const t_config Location::getConfig() const {
    t_config config = {
        autoindex,
        clientBodySize,
        redirect,
        root,
        index,
        methods,
        errorPages,
        cgiPaths
    };
    return (config);
}


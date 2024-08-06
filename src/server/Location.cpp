#include "Location.hpp"

Location::Location() : logger(Logger("LOCATION")), path(""), root(""), index(""), redirect(""), clientBodySize(0), methods(std::vector<Method>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

Location::Location(const LocationConfig& locationConfig) {
    logger = Logger("LOCATION");
    path = locationConfig.getPath();
    root = locationConfig.getRoot();
    index = locationConfig.getIndex();
    redirect = locationConfig.getRedirect();
    clientBodySize = locationConfig.getClientBodySize();
    methods = locationConfig.getMethods();
    errorPages = locationConfig.getErrorPages();
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
    }
    return (*this);
}

Location::~Location() {}

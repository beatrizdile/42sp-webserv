#include "LocationConfig.hpp"

#include <algorithm>

const size_t LocationConfig::DEFAULT_CLIENT_BODY_SIZE = 1000000;
const std::string LocationConfig::INDEX_KEY = "index";
const std::string LocationConfig::ROOT_KEY = "root";
const std::string LocationConfig::REDIRECT_KEY = "redirect";
const std::string LocationConfig::CLIENT_BODY_SIZE_KEY = "client_max_body_size";
const std::string LocationConfig::ALLOW_METHODS_KEY = "allow_methods";
const std::string LocationConfig::ERROR_PAGE_KEY = "error_page";
const std::string LocationConfig::AUTOINDEX_KEY = "autoindex";

LocationConfig::LocationConfig() : logger(Logger("LOCATION_CONFIG")), path(""), root(""), index(""), redirect(""), clientBodySize(DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>()), errorPages(std::vector<std::pair<size_t, std::string> >()), autoindex(false) {}

LocationConfig::LocationConfig(const LocationConfig& other) {
    *this = other;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
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
    }
    return (*this);
}

LocationConfig::~LocationConfig() {}

void LocationConfig::parseLocation(const AstNode& node) {
    if (node.getIsLeaf()) {
        throw std::runtime_error("Location block is empty at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Location block has invalid values at line: " + numberToString(node.getKey().getLine()));
    }
    path = node.getValues().front().getValue();

    std::vector<AstNode*> children = node.getChildren();
    if (children.size() == 0) {
        throw std::runtime_error("Location block is empty at line: " + numberToString(node.getKey().getLine()));
    }

    for (std::vector<AstNode*>::iterator it = children.begin(); it != children.end(); ++it) {
        std::string attribute = (*it)->getKey().getValue();

        if (attribute == LocationConfig::ROOT_KEY) {
            parseRoot(*(*it));
        } else if (attribute == LocationConfig::INDEX_KEY) {
            parseIndex(*(*it));
        } else if (attribute == LocationConfig::CLIENT_BODY_SIZE_KEY) {
            parseClientBodySize(*(*it));
        } else if (attribute == LocationConfig::ALLOW_METHODS_KEY) {
            parseMethod(*(*it));
        } else if (attribute == LocationConfig::ERROR_PAGE_KEY) {
            parseErrorPage(*(*it));
        } else if (attribute == LocationConfig::AUTOINDEX_KEY) {
            parseAutoindex(*(*it));
        } else {
            throw std::runtime_error("Unknown attribute '" + attribute + "' in server block at line: " + numberToString(node.getKey().getLine()));
        }
    }
}

void LocationConfig::parseRoot(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Root attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Root attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    root = node.getValues().front().getValue();
}

void LocationConfig::parseIndex(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Index attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Index attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    index = node.getValues().front().getValue();
}

void LocationConfig::parseRedirect(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Redirect attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Redirect attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    redirect = node.getValues().front().getValue();
}

void LocationConfig::parseClientBodySize(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Client body size attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Client body size attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    std::string value = node.getValues().front().getValue();
    char* end;
    long size = std::strtol(value.c_str(), &end, 10);
    if (*end != '\0' || size < 0) {
        throw std::runtime_error("Client body size attribute must be a number at line: " + numberToString(node.getKey().getLine()));
    }

    clientBodySize = size;
}

void LocationConfig::parseMethod(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Method attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    std::vector<Token> values = node.getValues();
    if (values.size() < 1) {
        throw std::runtime_error("Method attribute must have at least one values at line: " + numberToString(node.getKey().getLine()));
    }

    for (std::vector<Token>::iterator it = values.begin(); it != values.end(); ++it) {
        Method method = getMethodFromString((*it).getValue());
        if (method == INVALID) {
            throw std::runtime_error("Invalid method: '" + (*it).getValue() + "' at line: " + numberToString((*it).getLine()));
        }

        if (std::find(methods.begin(), methods.end(), method) == methods.end()) {
            methods.push_back(method);
        } else {
            throw std::runtime_error("Method '" + (*it).getValue() + "' already exists at line: " + numberToString((*it).getLine()));
        }
    }
}

void LocationConfig::parseErrorPage(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Error page attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    std::vector<Token> elems = node.getValues();
    if (elems.size() != 2) {
        throw std::runtime_error("Error page attribute must have two values at line: " + numberToString(node.getKey().getLine()));
    }

    char* end;
    long code = std::strtol(elems[0].getValue().c_str(), &end, 10);
    if (*end != '\0' || code < 100 || code > 599) {
        throw std::runtime_error("Error page code must be a number in valid range at line: " + numberToString(elems[0].getLine()));
    }

    errorPages.push_back(std::make_pair(code, elems[1].getValue()));
}

void LocationConfig::parseAutoindex(const AstNode& node) {
    if (!node.getIsLeaf()) {
        throw std::runtime_error("Autoindex attribute can't have children at line: " + numberToString(node.getKey().getLine()));
    }

    if (node.getValues().size() != 1) {
        throw std::runtime_error("Autoindex attribute expected one value at line: " + numberToString(node.getKey().getLine()));
    }

    std::string value = node.getValues().front().getValue();
    if (value == "on") {
        autoindex = true;
    } else if (value == "off") {
        autoindex = false;
    } else {
        throw std::runtime_error("Autoindex attribute must be 'on' or 'off' at line: " + numberToString(node.getKey().getLine()));
    }
}

const std::string& LocationConfig::getPath() const {
    return (path);
}

const std::string& LocationConfig::getRoot() const {
    return (root);
}

const std::string& LocationConfig::getIndex() const {
    return (index);
}

const std::string& LocationConfig::getRedirect() const {
    return (redirect);
}

size_t LocationConfig::getClientBodySize() const {
    return (clientBodySize);
}

const std::vector<Method>& LocationConfig::getMethods() const {
    return (methods);
}

const std::vector<std::pair<size_t, std::string> >& LocationConfig::getErrorPages() const {
    return (errorPages);
}

bool LocationConfig::getAutoindex() const {
    return (autoindex);
}

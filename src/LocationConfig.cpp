#include "LocationConfig.hpp"

#include <algorithm>

size_t LocationConfig::DEFAULT_CLIENT_BODY_SIZE = 1000000;
std::string LocationConfig::INDEX_KEY = "index";
std::string LocationConfig::ROOT_KEY = "root";
std::string LocationConfig::REDIRECT_KEY = "redirect";
std::string LocationConfig::CLIENT_BODY_SIZE_KEY = "client_max_body_size";
std::string LocationConfig::ALLOW_METHODS_KEY = "allow_methods";
std::string LocationConfig::ERROR_PAGE_KEY = "error_page";

LocationConfig::LocationConfig() : logger(Logger("LOCATION_CONFIG")), path(""), root(""), index(""), redirect(""), clientBodySize(DEFAULT_CLIENT_BODY_SIZE), methods(std::vector<Method>()), errorPages(std::vector<std::pair<size_t, std::string> >()) {}

LocationConfig::LocationConfig(const LocationConfig &other) {
    *this = other;
}

LocationConfig &LocationConfig::operator=(const LocationConfig &other) {
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

LocationConfig::~LocationConfig() {}

bool LocationConfig::parseLocationBlock(const std::string &locationBlockString, const std::string &locationPath) {
    path = locationPath;
    size_t startBlock = 0;
    size_t lastEndBlock = 0;

    while ((startBlock = locationBlockString.find(";", startBlock)) != std::string::npos) {
        std::string line = locationBlockString.substr(lastEndBlock, startBlock - lastEndBlock);
        std::vector<std::string> elems;

        trim(line);
        split(line, ' ', elems);

        if (!parseAttribute(elems)) {
            return (false);
        }

        startBlock++;
        lastEndBlock = startBlock;
    }

    if (methods.size() == 0) {
        methods.push_back(GET);
    }

    if (index.empty()) {
        index = "index.html";
    }

    return (true);
}

bool LocationConfig::parseAttribute(const std::vector<std::string> &elems) {
    if (elems.size() == 0) {
        return (false);
    }

    if (elems[0] == LocationConfig::ROOT_KEY) {
        return (processRoot(elems));
    } else if (elems[0] == LocationConfig::INDEX_KEY) {
        return (processIndex(elems));
    } else if (elems[0] == LocationConfig::REDIRECT_KEY) {
        return (processRedirect(elems));
    } else if (elems[0] == LocationConfig::CLIENT_BODY_SIZE_KEY) {
        return (processClientBodySize(elems));
    } else if (elems[0] == LocationConfig::ALLOW_METHODS_KEY) {
        return (processMethod(elems));
    } else if (elems[0] == LocationConfig::ERROR_PAGE_KEY) {
        return (processErrorPage(elems));
    }

    return (false);
}

bool LocationConfig::processRoot(const std::vector<std::string> &elems) {
    if (elems.size() != 2) {
        logger.error() << "Root attribute must have a value" << std::endl;
        return (false);
    }

    root = elems[1];
    return (true);
}

bool LocationConfig::processIndex(const std::vector<std::string> &elems) {
    if (elems.size() != 2) {
        logger.error() << "Index attribute must have a value" << std::endl;
        return (false);
    }

    index = elems[1];
    return (true);
}

bool LocationConfig::processRedirect(const std::vector<std::string> &elems) {
    if (elems.size() != 2) {
        logger.error() << "Redirect attribute must have a value" << std::endl;
        return (false);
    }

    redirect = elems[1];
    return (true);
}

bool LocationConfig::processClientBodySize(const std::vector<std::string> &elems) {
    if (elems.size() != 2) {
        logger.error() << "Client body size attribute must have a value" << std::endl;
        return (false);
    }

    char *end;
    long bodySize = std::strtol(elems[1].c_str(), &end, 10);
    if (*end != '\0' || bodySize < 0) {
        logger.error() << "Error code must be a number in valid range" << std::endl;
        return (false);
    }

    clientBodySize = bodySize;
    return (true);
}

bool LocationConfig::processMethod(const std::vector<std::string> &elems) {
    if (elems.size() < 2) {
        logger.error() << "Method attribute must have a value" << std::endl;
        return (false);
    }

    for (size_t i = 1; i < elems.size(); i++) {
        Method method = getMethod(elems[1]);
        if (method == INVALID) {
            logger.error() << "Invalid method '" << elems[1] << "'" << std::endl;
            return (false);
        }

        if (std::find(methods.begin(), methods.end(), method) == methods.end()) {
            methods.push_back(method);
        } else {
            logger.error() << "Method already exists: " << elems[i] << std::endl;
            return false;
        }
    }
    return (true);
}

bool LocationConfig::processErrorPage(const std::vector<std::string> &elems) {
    if (elems.size() != 3) {
        logger.error() << "Error page attribute must have a value" << std::endl;
        return (false);
    }

    char *end;
    long errorCode = std::strtol(elems[1].c_str(), &end, 10);
    if (*end != '\0' || errorCode < 100 || errorCode > 599) {
        logger.error() << "Error code must be a number in valid range" << std::endl;
        return (false);
    }

    errorPages.push_back(std::make_pair(errorCode, elems[2]));
    return (true);
}

void LocationConfig::printConfig() {
    logger.info() << "Location config -----------------" << std::endl;
    logger.info() << "Path: " << path << std::endl;
    logger.info() << "Root: " << root << std::endl;
    logger.info() << "Index: " << index << std::endl;
    logger.info() << "Redirect: " << redirect << std::endl;
    logger.info() << "Client body size: " << clientBodySize << std::endl;

    std::string methodsString;
    for (size_t i = 0; i < methods.size(); i++) {
        methodsString += getMethodString(methods[i]) + " ";
    }
    logger.info() << "Methods: " << methodsString << std::endl;

    std::string errorPagesString;
    for (size_t i = 0; i < errorPages.size(); i++) {
        errorPagesString += errorPages[i].first + " " + errorPages[i].second + " ";
    }
    logger.info() << "Error pages: " << errorPagesString << std::endl;
}

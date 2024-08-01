#pragma once

#include <string>
#include <vector>

#include "Logger.hpp"
#include "Method.hpp"
#include "utils.h"

class LocationConfig {
   public:
    static size_t DEFAULT_CLIENT_BODY_SIZE;
    static std::string INDEX_KEY;
    static std::string ROOT_KEY;
    static std::string REDIRECT_KEY;
    static std::string CLIENT_BODY_SIZE_KEY;
    static std::string ALLOW_METHODS_KEY;
    static std::string ERROR_PAGE_KEY;

    LocationConfig();
    LocationConfig(const LocationConfig &other);
    LocationConfig &operator=(const LocationConfig &other);
    ~LocationConfig();

    bool parseLocationBlock(const std::string &locationBlockString, const std::string &locationPath);
    void printConfig();

   private:
    Logger logger;

    std::string path;
    std::string root;
    std::string index;
    std::string redirect;
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<std::pair<size_t, std::string> > errorPages;

    bool parseAttribute(const std::vector<std::string> &elems);
    bool processRoot(const std::vector<std::string> &elems);
    bool processIndex(const std::vector<std::string> &elems);
    bool processRedirect(const std::vector<std::string> &elems);
    bool processClientBodySize(const std::vector<std::string> &elems);
    bool processMethod(const std::vector<std::string> &elems);
    bool processErrorPage(const std::vector<std::string> &elems);
};

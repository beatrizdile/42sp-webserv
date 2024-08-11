#pragma once

#include <string>
#include <vector>

#include "AstNode.hpp"
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
    LocationConfig(const LocationConfig& other);
    LocationConfig& operator=(const LocationConfig& other);
    ~LocationConfig();

    void parseLocation(const AstNode& node);

    std::string getPath() const;
    std::string getRoot() const;
    std::string getIndex() const;
    std::string getRedirect() const;
    size_t getClientBodySize() const;
    std::vector<Method> getMethods() const;
    std::vector<std::pair<size_t, std::string> > getErrorPages() const;

   private:
    Logger logger;

    std::string path;
    std::string root;
    std::string index;
    std::string redirect;
    // add autoindex
    size_t clientBodySize;
    std::vector<Method> methods;
    std::vector<std::pair<size_t, std::string> > errorPages;

    void parseRoot(const AstNode& node);
    void parseIndex(const AstNode& node);
    void parseRedirect(const AstNode& node);
    void parseClientBodySize(const AstNode& node);
    void parseMethod(const AstNode& node);
    void parseErrorPage(const AstNode& node);
};

#pragma once

#include <string>
#include <vector>
#include <map>

#include "AstNode.hpp"
#include "Logger.hpp"
#include "Method.hpp"
#include "utils.h"

class LocationConfig {
   public:
    static const size_t DEFAULT_CLIENT_BODY_SIZE;
    static const std::string DEFAULT_INDEX;
    static const std::string INDEX_KEY;
    static const std::string ROOT_KEY;
    static const std::string REDIRECT_KEY;
    static const std::string CLIENT_BODY_SIZE_KEY;
    static const std::string ALLOW_METHODS_KEY;
    static const std::string ERROR_PAGE_KEY;
    static const std::string AUTOINDEX_KEY;
    static const std::string CGI_PATH_KEY;

    LocationConfig();
    LocationConfig(const LocationConfig& other);
    LocationConfig& operator=(const LocationConfig& other);
    ~LocationConfig();

    void parseLocation(const AstNode& node);

    const std::string& getPath() const;
    const std::string& getRoot() const;
    const std::string& getIndex() const;
    const std::string& getRedirect() const;
    const std::map<std::string, std::string>& getCgiPaths() const;
    size_t getClientBodySize() const;
    const std::vector<Method>& getMethods() const;
    const std::vector<std::pair<size_t, std::string> >& getErrorPages() const;
    bool getAutoindex() const;

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
    std::map<std::string, std::string> cgiPaths;

    void parseRoot(const AstNode& node);
    void parseIndex(const AstNode& node);
    void parseRedirect(const AstNode& node);
    void parseClientBodySize(const AstNode& node);
    void parseMethod(const AstNode& node);
    void parseErrorPage(const AstNode& node);
    void parseAutoindex(const AstNode& node);
    void parseCgiPath(const AstNode& node);
};

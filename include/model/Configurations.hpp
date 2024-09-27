#pragma once

#include <map>
#include <string>
#include <vector>

#include "Method.hpp"

class Configurations {
   public:
    Configurations();
    Configurations(bool isAutoindex, size_t clientBodySize, const std::string& redirect, const std::string& root, const std::string& index, const std::vector<Method>& methods, const std::vector<std::pair<size_t, std::string> >& errorPages, const std::map<std::string, std::string>& cgiPaths);
    Configurations(const Configurations& other);
    Configurations& operator=(const Configurations& other);
    ~Configurations();

    bool getIsAutoindex() const;
    size_t getClientBodySize() const;
    const std::string& getRedirect() const;
    const std::string& getRoot() const;
    const std::string& getIndex() const;
    const std::vector<Method>& getMethods() const;
    const std::vector<std::pair<size_t, std::string> >& getErrorPages() const;
    const std::map<std::string, std::string>& getCgiPaths() const;

   private:
    bool isAutoindex;
    size_t clientBodySize;
    std::string redirect;
    std::string root;
    std::string index;
    std::vector<Method> methods;
    std::vector<std::pair<size_t, std::string> > errorPages;
    std::map<std::string, std::string> cgiPaths;
};

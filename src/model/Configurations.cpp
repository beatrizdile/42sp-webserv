#include "Configurations.hpp"

Configurations::Configurations() : isAutoindex(false), clientBodySize(0), redirect(""), root(""), index(""), methods(), errorPages(), cgiPaths() {}

Configurations::Configurations(bool isAutoindex, size_t clientBodySize, const std::string& redirect, const std::string& root, const std::string& index, const std::vector<Method>& methods, const std::vector<std::pair<size_t, std::string> >& errorPages, const std::map<std::string, std::string>& cgiPaths) : isAutoindex(isAutoindex), clientBodySize(clientBodySize), redirect(redirect), root(root), index(index), methods(methods), errorPages(errorPages), cgiPaths(cgiPaths) {}

Configurations::Configurations(const Configurations& other) : isAutoindex(other.isAutoindex), clientBodySize(other.clientBodySize), redirect(other.redirect), root(other.root), index(other.index), methods(other.methods), errorPages(other.errorPages), cgiPaths(other.cgiPaths) {}

Configurations& Configurations::operator=(const Configurations& other) {
    if (this != &other) {
        isAutoindex = other.isAutoindex;
        clientBodySize = other.clientBodySize;
        redirect = other.redirect;
        root = other.root;
        index = other.index;
        methods = other.methods;
        errorPages = other.errorPages;
        cgiPaths = other.cgiPaths;
    }
    return *this;
}

Configurations::~Configurations() {}

bool Configurations::getIsAutoindex() const { return isAutoindex; }
size_t Configurations::getClientBodySize() const { return clientBodySize; }
const std::string& Configurations::getRedirect() const { return redirect; }
const std::string& Configurations::getRoot() const { return root; }
const std::string& Configurations::getIndex() const { return index; }
const std::vector<Method>& Configurations::getMethods() const { return methods; }
const std::vector<std::pair<size_t, std::string> >& Configurations::getErrorPages() const { return errorPages; }
const std::map<std::string, std::string>& Configurations::getCgiPaths() const { return cgiPaths; }

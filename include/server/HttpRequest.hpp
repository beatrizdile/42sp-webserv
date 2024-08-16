#pragma once

#include <map>
#include <string>

#include "Logger.hpp"
#include "Method.hpp"

class HttpRequest {
   public:
    HttpRequest();
    HttpRequest(const HttpRequest &copy);
    ~HttpRequest();
    HttpRequest &operator=(const HttpRequest &assign);

    bool digestRequest(const std::string &data);
    void clear();

    Method getMethod() const;
    const std::string &getUri() const;
    const std::string &getVersion() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getBody() const;
    bool isComplete() const;

   private:
    Logger logger;

    std::string rawData;
    Method method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t contentLength;
    bool complete;

    void parseFristLine();
    void parseHeaders(size_t endPos);
};

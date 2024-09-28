#pragma once

#include <map>
#include <string>

#include "Logger.hpp"
#include "Method.hpp"

class HttpRequest {
   public:
    static const std::string HEADER_HOST_KEY;
    static const std::string URI_CHARACTERS;
    static const std::string HEADER_CONTENT_TYPE_KEY;

    HttpRequest();
    HttpRequest(const HttpRequest &copy);
    ~HttpRequest();
    HttpRequest &operator=(const HttpRequest &assign);

    bool digestRequest(const std::string &data);
    void clear();

    Method getMethod() const;
    const std::string &getUri() const;
    const std::string &getQueryParameters() const;
    const std::string &getVersion() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getBody() const;
    bool isComplete() const;
    std::string getEtag() const;
    static bool verifyHeaderKey(const std::string &key);
    static bool verifyHeaderValue(const std::string &value);

   private:
    Logger logger;

    std::string rawData;
    Method method;
    std::string uri;
    std::string queryParameters;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t contentLength;
    bool complete;

    static const std::string HEADER_VALUE_CHARACTERS;
    static const std::string HEADER_KEY_CHARACTERS;
    static const std::string CONTENT_LENTH;
    static const std::string HTTP_VERSION;
    static const std::string CONTENT_LENTH_HEADER_KEY;
    static const std::string HEADER_ETAG_KEY;

    void parseFristLine();
    void parseHeaders(size_t endPos);
};

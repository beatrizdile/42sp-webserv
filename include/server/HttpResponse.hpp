#pragma once

#include <string>

class HttpResponse {
   private:
    static const std::string serverName;
    static const std::string httpVersion;

    int httpStatus;
    std::string contentType;
    std::string body;
    std::string lastModified;

    std::string createDate();
    std::string getStatusMessage();

   public:
    HttpResponse();
    HttpResponse(const HttpResponse &copy);
    ~HttpResponse();
    HttpResponse &operator=(const HttpResponse &assign);

    std::string createResponse();
    void clear();
    void generateDefaultErrorPage();

    void setHttpStatus(int status);
    void setContentType(const std::string &type);
    void setBody(const std::string &body);
    void setLastModified(const std::string &lastModified);
};

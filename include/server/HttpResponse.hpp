#pragma once

#include <string>

class HttpResponse {
   private:
    static const std::string SERVER_NAME;
    static const std::string HTTP_VERSION;
    static const std::string DEFAULT_MIME_TYPE;

    int httpStatus;
    std::string contentType;
    std::string body;
    std::string lastModified;
    std::string fileName;

    std::string createDate();
    std::string getStatusMessage();
    std::string setContentTypeFromFilename();

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
    void setFileName(const std::string &fileName);
    void createAutoindex(std::string directoryPat);
};

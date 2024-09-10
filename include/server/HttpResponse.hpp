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
    std::string etag;
    std::string location;
    bool hasZeroContentLength;

    std::string createDate();
    std::string getStatusMessage();
    std::string setContentTypeFromFilename();
    void createAutoindex(const std::string &directoryPath, const std::string &uri);
    void clear();
    void generateDefaultErrorPage();
    std::string createResponse();
    void generateEtag(const struct stat &fileInfo);

   public:
    HttpResponse();
    HttpResponse(const HttpResponse &copy);
    ~HttpResponse();
    HttpResponse &operator=(const HttpResponse &assign);

    std::string createResponseFromStatus(int status);
    std::string createResponseFromLocation(int status, const std::string &location);
    std::string createIndexResponse(const std::string &directoryPath, const std::string &uri);
    std::string createFileResponse(const std::string &filePath, const std::string &etag);
};

#pragma once

#include <map>
#include <string>
#include <vector>

class HttpResponse {
   private:
    static const std::string SERVER_NAME;
    static const std::string HTTP_VERSION;
    static const std::string DEFAULT_MIME_TYPE;

    size_t httpStatus;
    std::string contentType;
    std::string body;
    std::string lastModified;
    std::string fileName;
    std::string etag;
    std::string location;
    bool hasZeroContentLength;
    std::map<std::string, std::string> extraHeaders;

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

    std::string createResponseFromStatus(size_t status);
    std::string createResponseFromLocation(size_t status, const std::string &location);
    std::string createCgiResponse(size_t status, const std::string &body, const std::map<std::string, std::string> &headers);
    std::string createErrorResponse(size_t status, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages);
    std::string createFileResponse(const std::string &filePath, const std::string &etag, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages);
    std::string createIndexResponse(const std::string &directoryPath, const std::string &uri, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages);
};

#include "HttpResponse.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

const std::string HttpResponse::SERVER_NAME = "Webserver/1.0";
const std::string HttpResponse::HTTP_VERSION = "HTTP/1.1";
const std::string HttpResponse::DEFAULT_MIME_TYPE = "text/plain";

HttpResponse::HttpResponse() : httpStatus(0), contentType(""), body(""), lastModified(""), fileName("") {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &copy) {
    *this = copy;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &assign) {
    if (this != &assign) {
        httpStatus = assign.httpStatus;
        contentType = assign.contentType;
        body = assign.body;
        lastModified = assign.lastModified;
        fileName = assign.fileName;
    }

    return *this;
}

std::string HttpResponse::createResponse() {
    std::ostringstream serverResponse;

    createAutoindex("/Users/beatrizdile/Documents/nginx-test");

    if (httpStatus >= 400 && httpStatus <= 500 && body.empty()) {
        generateDefaultErrorPage();
    }

    serverResponse << HTTP_VERSION << " " << httpStatus << " " << getStatusMessage() << "\r\n";
    serverResponse << "Server: " << SERVER_NAME << "\r\n";
    serverResponse << "Date: " << createDate() << "\r\n";

    if (!contentType.empty())
        serverResponse << "Content-Type: " << contentType << "\r\n";

    if (!body.empty())
        serverResponse << "Content-Length: " << body.size() << "\r\n";

    if (!lastModified.empty())
        serverResponse << "Last-Modified: " << lastModified << "\r\n";

    if (!fileName.empty())
        serverResponse << "Content-Type: " << setContentTypeFromFilename() << "\r\n";

    serverResponse << "\r\n";

    if (!body.empty()) {
        serverResponse << body;
    }

    clear();

    return serverResponse.str();
}

void HttpResponse::clear() {
    httpStatus = 0;
    contentType.clear();
    body.clear();
    lastModified.clear();
}

std::string HttpResponse::createDate() {
    std::time_t now = std::time(0);
    std::tm *gmt_time = std::gmtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt_time);
    return buffer;
}

void HttpResponse::generateDefaultErrorPage() {
    std::ostringstream errorPage;

    errorPage << "<html>\n";
    errorPage << "<head><title>" << httpStatus << " " << getStatusMessage() << "</title></head>\n";
    errorPage << "<body>\n";
    errorPage << "<center><h1>" << httpStatus << " " << getStatusMessage() << "</h1></center>\n";
    errorPage << "<hr><center>" << SERVER_NAME << "</center>\n";
    errorPage << "</body>\n";
    errorPage << "</html>\n";

    body = errorPage.str();
    contentType = "text/html";
}

void HttpResponse::setHttpStatus(int status) {
    httpStatus = status;
}

void HttpResponse::setContentType(const std::string &type) {
    contentType = type;
}

void HttpResponse::setBody(const std::string &content) {
    body = content;
}

void HttpResponse::setLastModified(const std::string &date) {
    lastModified = date;
}

void HttpResponse::setFileName(const std::string &name) {
    fileName = name;
}

std::string HttpResponse::getStatusMessage() {
    switch (httpStatus) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 204:
            return "No Content";
        case 206:
            return "Partial Content";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 413:
            return "Payload Too Large";
        case 414:
            return "URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        default:
            return "Unknown Status Code";
    }
}

std::string HttpResponse::setContentTypeFromFilename() {
    static const std::pair<const char *, const char *> mimeTypesArray[] = {
        std::make_pair("html", "text/html"),
        std::make_pair("htm", "text/html"),
        std::make_pair("txt", "text/plain"),
        std::make_pair("css", "text/css"),
        std::make_pair("js", "application/javascript"),
        std::make_pair("json", "application/json"),
        std::make_pair("xml", "application/xml"),
        std::make_pair("jpg", "image/jpeg"),
        std::make_pair("jpeg", "image/jpeg"),
        std::make_pair("png", "image/png"),
        std::make_pair("gif", "image/gif"),
        std::make_pair("bmp", "image/bmp"),
        std::make_pair("webp", "image/webp"),
        std::make_pair("svg", "image/svg+xml"),
        std::make_pair("ico", "image/x-icon"),
        std::make_pair("mp4", "video/mp4"),
        std::make_pair("avi", "video/x-msvideo"),
        std::make_pair("mov", "video/quicktime")};

    std::string extension = "";
    std::size_t dotPos = fileName.rfind('.');
    if (dotPos != std::string::npos) {
        extension = fileName.substr(dotPos + 1);
    }
    std::string mimeType = DEFAULT_MIME_TYPE;
    for (std::size_t i = 0; i < sizeof(mimeTypesArray) / sizeof(mimeTypesArray[0]); ++i) {
        if (extension == mimeTypesArray[i].first) {
            mimeType = mimeTypesArray[i].second;
            break;
        }
    }

    return (mimeType);
}

std::string getFileModificationDate(std::string filePath) {
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        return "";
    }

    std::time_t modTime = fileInfo.st_mtime;
    struct tm *gmtTime = std::gmtime(&modTime);
    char timeString[80];
    std::strftime(timeString, sizeof(timeString), "%a, %d %b %Y %H:%M:%S", gmtTime);
    return timeString;
}

void HttpResponse::createAutoindex(std::string directoryPath) {
    DIR *dir = opendir(directoryPath.c_str());
    if (dir == NULL) {
        httpStatus = 404;
        return;
    }
    std::ostringstream indexPage;
    struct dirent *dent;

    indexPage << "<html>\n";
    indexPage << "<head><title> Index of " << directoryPath << "</title></head>\n";
    indexPage << "<body><h1>Index of " << directoryPath << "</h1><hr><pre>\n";
    indexPage << "<a href='../'>../</a>\n";

    while ((dent = readdir(dir)) != NULL) {
        std::string strName = dent->d_name;
        if (strName == "." || strName == "..") {
            continue;
        }

        indexPage << "<a href='" << strName << "'>" << std::setw(50) << std::left << strName + "</a>";

        std::string date = getFileModificationDate(directoryPath + "/" + strName);
        if (date.empty()) {
            httpStatus = 500;
            return;
        }
        indexPage << date;

        if (dent->d_type == DT_DIR)
            indexPage << std::setw(30) << std::right << "-" << "\n";
        else
            indexPage << std::setw(30) << std::right << dent->d_reclen << "\n";
    }
    indexPage << "</pre><hr></body></html>";

    httpStatus = 200;
    fileName = "index.html";
    body = indexPage.str();
}

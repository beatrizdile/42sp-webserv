#include "HttpResponse.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "utils.h"

const std::string HttpResponse::SERVER_NAME = "Webserver/1.0";
const std::string HttpResponse::HTTP_VERSION = "HTTP/1.1";
const std::string HttpResponse::DEFAULT_MIME_TYPE = "text/plain";

HttpResponse::HttpResponse() : httpStatus(0), contentType(""), body(""), lastModified(""), fileName(""), etag(""), hasZeroContentLength(false), extraHeaders(), cookies() {}

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
        etag = assign.etag;
        location = assign.location;
        hasZeroContentLength = assign.hasZeroContentLength;
        extraHeaders = assign.extraHeaders;
        cookies = assign.cookies;
    }

    return *this;
}

std::string HttpResponse::createResponse() {
    std::ostringstream serverResponse;

    if (httpStatus >= 400 && httpStatus <= 500 && body.empty()) {
        generateDefaultErrorPage();
    }

    serverResponse << HTTP_VERSION << ' ' << httpStatus << ' ' << getStatusMessage() << "\r\n";
    serverResponse << "Server: " << SERVER_NAME << "\r\n";
    serverResponse << "Date: " << createDate() << "\r\n";

    if (!contentType.empty())
        serverResponse << "Content-Type: " << contentType << "\r\n";

    if (!extraHeaders.empty()) {
        for (std::map<std::string, std::string>::const_iterator it = extraHeaders.begin(); it != extraHeaders.end(); ++it) {
            serverResponse << it->first << ": " << it->second << "\r\n";
        }
    }

    if ((!body.empty() || hasZeroContentLength) && extraHeaders.find("Content-Length") == extraHeaders.end())
        serverResponse << "Content-Length: " << (hasZeroContentLength ? 0 : body.size()) << "\r\n";

    if (!lastModified.empty())
        serverResponse << "Last-Modified: " << lastModified << "\r\n";

    if (!fileName.empty())
        serverResponse << "Content-Type: " << setContentTypeFromFilename() << "\r\n";

    if (!location.empty())
        serverResponse << "Location: " << location << "\r\n";

    if (!etag.empty())
        serverResponse << "ETag: " << etag << "\r\n";

    if (!cookies.empty()) {
        for (std::vector<std::string>::const_iterator cookie = cookies.begin(); cookie != cookies.end(); ++cookie) {
            serverResponse << "Set-Cookie: " << *cookie << "\r\n";
        }
    }

    serverResponse << "\r\n";

    if (!body.empty()) {
        serverResponse << body;
    }

    return serverResponse.str();
}

std::string HttpResponse::createResponseFromLocation(size_t status, const std::string &location) {
    httpStatus = status;
    this->location = location;
    this->hasZeroContentLength = true;
    std::string responseString = createResponse();
    clear();
    return (responseString);
}

void HttpResponse::clear() {
    httpStatus = 0;
    contentType.clear();
    body.clear();
    lastModified.clear();
    fileName.clear();
    etag.clear();
    location.clear();
    extraHeaders.clear();
    cookies.clear();
    hasZeroContentLength = false;
}

void HttpResponse::setCookie(const std::string &key, const std::string &value, const std::string &expires, const std::string &path = "/", bool httpOnly = false) {
    std::string cookie = key + "=" + value + "; Expires=" + expires + "; Path=" + path;
    if (httpOnly) {
        cookie += "; HttpOnly";
    }
    cookies.push_back(cookie);
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
    errorPage << "<head><title>" << httpStatus << ' ' << getStatusMessage() << "</title></head>\n";
    errorPage << "<body>\n";
    errorPage << "<center><h1>" << httpStatus << ' ' << getStatusMessage() << "</h1></center>\n";
    errorPage << "<hr><center>" << SERVER_NAME << "</center>\n";
    errorPage << "</body>\n";
    errorPage << "</html>\n";

    body = errorPage.str();
    contentType = "text/html";
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

std::string getFileModificationDate(const struct stat &fileInfo, bool gmt) {
    std::time_t modTime = fileInfo.st_mtime;
    struct tm *gmtTime = std::gmtime(&modTime);
    char timeString[80];
    std::strftime(timeString, sizeof(timeString), gmt ? "%a, %d %b %Y %H:%M:%S GMT" : "%a, %d %b %Y %H:%M:%S", gmtTime);
    return timeString;
}

void HttpResponse::createAutoindex(const std::string &directoryPath, const std::string &uri) {
    DIR *dir = opendir(directoryPath.c_str());
    if (dir == NULL) {
        httpStatus = 404;
        return;
    }
    std::ostringstream indexPage;
    struct dirent *dent;

    indexPage << "<html>\n";
    indexPage << "<head><title> Index of " << uri << "</title></head>\n";
    indexPage << "<body><h1>Index of " << uri << "</h1><hr><pre>\n";
    indexPage << "<a href='../'>../</a>\n";

    while ((dent = readdir(dir)) != NULL) {
        std::string strName = dent->d_name;
        if (strName[0] == '.') {
            continue;
        }

        indexPage << "<a href='" << strName + ((dent->d_type == DT_DIR) ? "/" : "") << "'>" << std::setw(50) << std::left << strName + "</a>";

        struct stat fileInfo;
        if (stat((directoryPath + '/' + strName).c_str(), &fileInfo) != 0) {
            httpStatus = 500;
            closedir(dir);
            return;
        }

        std::string date = getFileModificationDate(fileInfo, false);
        if (date.empty()) {
            httpStatus = 500;
            closedir(dir);
            return;
        }
        indexPage << date;

        if (dent->d_type == DT_DIR)
            indexPage << std::setw(30) << std::right << '-' << std::endl;
        else
            indexPage << std::setw(30) << std::right << fileInfo.st_size << std::endl;
    }
    closedir(dir);
    indexPage << "</pre><hr></body></html>";

    httpStatus = 200;
    fileName = "index.html";
    body = indexPage.str();
}

std::string HttpResponse::createResponseFromStatus(size_t status) {
    httpStatus = status;
    std::string responseString = createResponse();
    clear();
    return (responseString);
}

std::string HttpResponse::createCgiResponse(size_t status, const std::string &body, const std::map<std::string, std::string> &headers, const std::vector<std::string> &cookies) {
    httpStatus = status;
    this->body = body;
    hasZeroContentLength = body.empty();
    extraHeaders = headers;
    this->cookies = cookies;
    std::string responseString = createResponse();
    clear();
    return (responseString);
}

std::string HttpResponse::createErrorResponse(size_t status, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages) {
    httpStatus = status;
    for (std::vector<std::pair<size_t, std::string> >::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
        if (it->first == status) {
            std::string filePath = createPath(root, it->second);
            std::ifstream file(filePath.c_str());
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                file.close();
            }
        }
    }

    std::string responseString = createResponse();
    clear();
    return (responseString);
}

std::string HttpResponse::createIndexResponse(const std::string &directoryPath, const std::string &uri, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages) {
    createAutoindex(directoryPath, uri);
    if (httpStatus >= 400 && httpStatus <= 599) {
        return (createErrorResponse(httpStatus, root, errorPages));
    }
    std::string responseString = createResponse();
    clear();
    return (responseString);
}

void HttpResponse::generateEtag(const struct stat &fileInfo) {
    std::ostringstream etagStream;
    etagStream << std::hex << '"' << fileInfo.st_mtime << '-' << fileInfo.st_size << '"';
    etag = etagStream.str();
}

std::string HttpResponse::createFileResponse(const std::string &filePath, const std::string &etag, const std::string &root, const std::vector<std::pair<size_t, std::string> > &errorPages) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return (createErrorResponse(404, root, errorPages));
    } else {
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) != 0) {
            return (createErrorResponse(500, root, errorPages));
        } else {
            lastModified = getFileModificationDate(fileInfo, true);
            generateEtag(fileInfo);
            if (this->etag == etag) {
                httpStatus = 304;
            } else {
                std::stringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                this->fileName = filePath;
                httpStatus = 200;
            }
        }
        file.close();
    }

    std::string responseString = createResponse();
    clear();
    return (responseString);
}

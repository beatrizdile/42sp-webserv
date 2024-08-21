#include "HttpResponse.hpp"

#include <cstdio>
#include <ctime>
#include <sstream>

const std::string HttpResponse::serverName = "Webserver/1.0";
const std::string HttpResponse::httpVersion = "HTTP/1.1";

HttpResponse::HttpResponse() : httpStatus(0), contentType(""), body(""), lastModified("") {}

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
    }

    return *this;
}

std::string HttpResponse::createResponse() {
    std::ostringstream serverResponse;

    if (httpStatus >= 400 && httpStatus <= 500 && body.empty()) {
        generateDefaultErrorPage();
    }

    serverResponse << httpVersion << " " << httpStatus << " " << getStatusMessage() << "\r\n";
    serverResponse << "Server: " << serverName << "\r\n";
    serverResponse << "Date: " << createDate() << "\r\n";

    if (!contentType.empty())
        serverResponse << "Content-Type: " << contentType << "\r\n";

    if (!body.empty())
        serverResponse << "Content-Length: " << body.size() << "\r\n";

    if (!lastModified.empty())
        serverResponse << "Last-Modified: " << lastModified << "\r\n";

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
    errorPage << "<hr><center>" << serverName << "</center>\n";
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

// criar metodo para parsear o arquivo e dizer seu Content-Type

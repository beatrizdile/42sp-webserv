#include "HttpRequest.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "utils.h"

HttpRequest::HttpRequest() : logger("HTTP_REQUEST"), rawData(""), method(INVALID), uri(""), version(""), headers(), body(""), contentLength(0), complete(false) {}

HttpRequest::HttpRequest(const HttpRequest &copy) {
    *this = copy;
}

HttpRequest::~HttpRequest() {}

HttpRequest &HttpRequest::operator=(const HttpRequest &assign) {
    if (this != &assign) {
        logger = assign.logger;
        rawData = assign.rawData;
        method = assign.method;
        uri = assign.uri;
        version = assign.version;
        headers = assign.headers;
        body = assign.body;
        contentLength = assign.contentLength;
        complete = assign.complete;
    }
    return (*this);
}

void HttpRequest::clear() {
    rawData.clear();
    method = INVALID;
    uri.clear();
    version.clear();
    headers.clear();
    body.clear();
    contentLength = 0;
    complete = false;
}

bool HttpRequest::digestRequest(const std::string &data) {
    rawData += data;

    try {
        if (version.empty() && uri.empty() && method == INVALID) {
            parseFristLine();
        }

        size_t pos = rawData.find("\r\n\r\n");
        if (pos != std::string::npos) {
            parseHeaders(pos);
            if (contentLength == 0) {
                complete = true;
            }
        }

        if (contentLength > 0 && rawData.size() >= contentLength) {
            body = rawData.substr(0, contentLength);
            rawData = rawData.substr(contentLength);
            complete = true;
        }

        return (true);
    } catch (std::exception &e) {
        logger.error() << "Error: " << e.what() << std::endl;
        return (false);
    }
}

void HttpRequest::parseFristLine() {
    size_t pos = rawData.find("\r\n");
    if (pos == std::string::npos) {
        return;
    }

    std::string line = rawData.substr(0, pos);
    rawData = rawData.substr(pos + 2);

    std::istringstream ss(line);
    std::string stringMethod;
    ss >> stringMethod;
    ss >> uri;
    ss >> version;

    if (stringMethod.empty()) {
        throw std::runtime_error("Not found method");
    }
    if (uri.empty()) {
        throw std::runtime_error("Not found URI");
    }
    if (version.empty()) {
        throw std::runtime_error("Not found version");
    }

    std::string rest;
    ss >> rest;
    if (!rest.empty()) {
        throw std::runtime_error("Extra parameters in first line");
    }

    method = getMethodFromString(stringMethod);
    if (method == INVALID) {
        throw std::runtime_error("Invalid method found '" + stringMethod + "'");
    }

    if (uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVXWYZabcdefghijklmnopqrstuvxwyz0123456789-_.~/?:@&=+$,#") != std::string::npos) {
        throw std::runtime_error("Invalid caracter in URI");
    }

    if (version != "HTTP/1.1") {
        throw std::runtime_error("Invalid HTTP version '" + version + "'");
    }
}

void HttpRequest::parseHeaders(size_t endPos) {
    std::string data = rawData.substr(0, endPos + 2);
    rawData = rawData.substr(endPos + 4);

    size_t pos;
    while ((pos = data.find("\r\n")) != std::string::npos) {
        std::string line = data.substr(0, pos);
        data = data.substr(pos + 2);

        if ((pos = line.find(":")) == std::string::npos) {
            throw std::runtime_error("Invalid header '" + line + "'");
        }

        std::string key = line.substr(0, pos);
        trim(key);
        std::string value = line.substr(pos + 1);
        trim(value);

        headers[key] = value;
    }

    if (headers.find("Content-Length") != headers.end()) {
        char *end;
        long size = std::strtol(headers["Content-Length"].c_str(), &end, 10);
        if (*end != '\0' || size < 0) {
            throw std::runtime_error("Invalid Content-Length '" + headers["Content-Length"] + "'");
        }
        contentLength = size;
    }
}

Method HttpRequest::getMethod() const {
    return (method);
}

const std::string &HttpRequest::getUri() const {
    return (uri);
}

const std::string &HttpRequest::getVersion() const {
    return (version);
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
    return (headers);
}

const std::string &HttpRequest::getBody() const {
    return (body);
}

bool HttpRequest::isComplete() const {
    return (complete);
}

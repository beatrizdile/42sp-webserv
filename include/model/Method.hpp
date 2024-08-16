#pragma once

#include <string>

enum Method {
    HEAD,
    GET,
    POST,
    DELETE,
    OPTIONS,
    INVALID,
};

Method getMethodFromString(const std::string& method);
std::string getMethodString(Method method);

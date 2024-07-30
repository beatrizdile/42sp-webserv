#include "Method.hpp"

Method getMethod(const std::string& method) {
    if (method == "HEAD") {
        return HEAD;
    } else if (method == "GET") {
        return GET;
    } else if (method == "POST") {
        return POST;
    } else if (method == "DELETE") {
        return DELETE;
    } else if (method == "OPTIONS") {
        return OPTIONS;
    } else {
        return INVALID;
    }
}

std::string getMethodString(Method method) {
    switch (method) {
        case HEAD:
            return "HEAD";
        case GET:
            return "GET";
        case POST:
            return "POST";
        case DELETE:
            return "DELETE";
        case OPTIONS:
            return "OPTIONS";
        default:
            return "INVALID";
    }
}

#include "ServerConfig.hpp"

ServerConfig::ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig &other) {
    *this = other;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &other) {
    if (this != &other) {
        port = other.port;
        host = other.host;
        name = other.name;
        locations = other.locations;
    }
    return *this;
}

ServerConfig::~ServerConfig() {}

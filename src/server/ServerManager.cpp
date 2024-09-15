#include "ServerManager.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>

const size_t ServerManager::MAX_CLIENTS = 1000;
const size_t ServerManager::READ_BUFFER_SIZE = 1024;

ServerManager::ServerManager() : logger(Logger("SERVER_MANAGER")), socketFd(0), port(-1), host(INADDR_ANY), servers(std::vector<Server>()), clientSockets(std::vector<int>()), request(HttpRequest()), response(HttpResponse()) {}

ServerManager::ServerManager(const std::vector<ServerConfig>& serverConfig) {
    logger = Logger("SERVER_MANAGER");
    socketFd = 0;
    port = serverConfig.front().getPort();
    host = serverConfig.front().getHost();
    clientSockets = std::vector<int>();
    request = HttpRequest();
    response = HttpResponse();

    for (std::vector<ServerConfig>::const_iterator it = serverConfig.begin(); it != serverConfig.end(); ++it) {
        servers.push_back(Server(*it));
    }
}

ServerManager::ServerManager(const ServerManager& other) {
    *this = other;
}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    if (this != &other) {
        logger = other.logger;
        socketFd = other.socketFd;
        port = other.port;
        host = other.host;
        servers = other.servers;
        clientSockets = other.clientSockets;
        request = other.request;
        response = other.response;
    }
    return (*this);
}

ServerManager::~ServerManager() {}

int ServerManager::initServer() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

#ifndef __APPLE__
    int optval = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        close(socketFd);
        throw createError("setsockopt");
    }
#endif

    struct sockaddr_in server_addr;
    std::memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = host;
    server_addr.sin_port = htons(port);

    if (bind(socketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        throw createError("bind");
    };

    if (listen(socketFd, MAX_CLIENTS) == -1) {
        throw createError("listen");
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &host, ipStr, INET_ADDRSTRLEN);
    logger.info() << "ServerManager " << ipStr << " started on port " << port << std::endl;

    return (socketFd);
}

std::vector<int> ServerManager::finishServer() const {
    if (socketFd != 0)
        close(socketFd);

    for (std::vector<int>::const_iterator it = clientSockets.begin(); it != clientSockets.end(); ++it) {
        close(*it);
    }

    std::vector<int> allFds = clientSockets;
    allFds.push_back(socketFd);
    return (allFds);
}

int ServerManager::acceptConnection() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientFd = accept(socketFd, (struct sockaddr*)&clientAddress, &clientAddressLen);

    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags == -1) {
        close(clientFd);
        logger.perror("fcntl");
        return (-1);
    }

    if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(clientFd);
        logger.perror("fcntl");
        return (-1);
    }

    clientSockets.push_back(clientFd);
    return (clientFd);
}

int ServerManager::handleClient(int clientSocket) {
    char buffer[READ_BUFFER_SIZE];
    ssize_t bytesRead = 0;

    bytesRead = recv(clientSocket, buffer, READ_BUFFER_SIZE, 0);
    if (bytesRead == -1) {
        logger.perror("recv");
        return (removeClient(clientSocket));
    }

    if (bytesRead == 0) {
        logger.info() << "Client disconnected on fd " << clientSocket << std::endl;
        return (removeClient(clientSocket));
    }

    if (!request.digestRequest(std::string(buffer, bytesRead))) {
        std::string responseString = response.createResponseFromStatus(400);
        ssize_t bytesSend = send(clientSocket, responseString.c_str(), responseString.size(), 0);
        if (bytesSend == -1) {
            logger.perror("send");
            return (removeClient(clientSocket));
        }
        if (bytesSend != static_cast<ssize_t>(responseString.size())) {
            logger.error() << "Error: failed to send response on fd " << clientSocket << std::endl;
            return (removeClient(clientSocket));
        }

        logger.error() << "Error: failed to parse request on fd " << clientSocket << std::endl;
        return (removeClient(clientSocket));
    }

    if (request.isComplete()) {
        return (matchUriAndResponseClient(clientSocket));
    }

    return (0);
}

int ServerManager::matchUriAndResponseClient(int clientSocket) {
    logger.info() << "Request: " << getMethodString(request.getMethod()) << " " << request.getUri() << " " << request.getVersion() << std::endl;

    // Find server that match with "Host" header
    std::vector<Server>::const_iterator server = findServer(request.getHeaders().at(HttpRequest::HEADER_HOST_KEY));

    // Find location in server that match with URI
    std::vector<Location>::const_iterator location = (*server).matchUri(request.getUri());

    // Process request
    std::string responseString;
    if (location == (*server).getLocations().end()) {
        responseString = processRequest((*server).getConfig(), request.getUri(), request.getHeaders());
    } else {
        responseString = processRequest((*location).getConfig(), request.getUri(), request.getHeaders());
    }
    request.clear();

    ssize_t bytesSend = send(clientSocket, responseString.c_str(), responseString.size(), 0);
    if (bytesSend == -1) {
        logger.perror("send");
        return (removeClient(clientSocket));
    }
    if (bytesSend != static_cast<ssize_t>(responseString.size())) {
        logger.error() << "Error: failed to send response" << std::endl;
        return (removeClient(clientSocket));
    }

    return (0);
}

std::string ServerManager::processRequest(const t_config& config, const std::string& uri, const std::map<std::string, std::string>& headers) {
    if (config.redirect != "") {
        return (response.createResponseFromLocation(301, config.redirect));
    }

    std::string path = createPath(config.root, uri);

    if (std::find(config.methods.begin(), config.methods.end(), request.getMethod()) == config.methods.end()) {
        return (response.createErrorResponse(405, config.root, config.errorPages));
    }

    if (request.getBody().size() > config.clientBodySize) {
        return (response.createErrorResponse(413, config.root, config.errorPages));
    }

    if (request.getMethod() == GET) {
        return (processGetRequest(config, path, uri));
    } else if (request.getMethod() == POST) {
        return (processPostRequest(config, path, uri, headers));
    } else if (request.getMethod() == DELETE) {
        return (processDeleteRequest(config, path));
    }
    return (response.createErrorResponse(501, config.root, config.errorPages));
}

std::string ServerManager::processGetRequest(const t_config& config, const std::string& path, const std::string& uri) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        return (response.createErrorResponse(404, config.root, config.errorPages));
    }
    std::string etag = request.getEtag();

    if (S_ISDIR(fileStat.st_mode)) {
        if (path[path.size() - 1] != '/') {
            return (response.createResponseFromStatus(301));
        } else if (access((path + "/" + config.index).c_str(), F_OK) != -1) {
            return (response.createFileResponse(path + "/" + config.index, etag, config.root, config.errorPages));
        } else if (config.isAutoindex) {
            return (response.createIndexResponse(path, uri, config.root, config.errorPages));
        } else {
            return (response.createErrorResponse(403, config.root, config.errorPages));
        }
    } else if (S_ISREG(fileStat.st_mode)) {
        return (response.createFileResponse(path, etag, config.root, config.errorPages));
    } else {
        return (response.createErrorResponse(404, config.root, config.errorPages));
    }
}

std::string ServerManager::processPostRequest(const t_config& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers) {
    if (request.getBody().empty()) {
        return (response.createErrorResponse(400, config.root, config.errorPages));
    }

    std::map<std::string, std::string>::const_iterator it = headers.find(HttpRequest::HEADER_CONTENT_TYPE_KEY);
    const std::string& contentType = (it != headers.end()) ? it->second : "application/octet-stream";
    if (contentType != "text/plain" && contentType != "application/octet-stream") {
        return (response.createErrorResponse(415, config.root, config.errorPages));
    }

    if (access(path.c_str(), F_OK) != -1) {
        return (response.createErrorResponse(409, config.root, config.errorPages));
    }
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        return (response.createErrorResponse(500, config.root, config.errorPages));
    }

    file << request.getBody();
    file.close();

    return (response.createResponseFromLocation(201, uri));
}

static bool isDirectory(const std::string& path) {
    struct stat pathStat;

    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }

    return S_ISDIR(pathStat.st_mode);
}

std::string ServerManager::processDeleteRequest(const t_config& config, const std::string& path) {
    if (access(path.c_str(), F_OK) == -1) {
        return (response.createErrorResponse(404, config.root, config.errorPages));
    }
    if (access(path.c_str(), W_OK) == -1) {
        return (response.createErrorResponse(403, config.root, config.errorPages));
    }

    if (isDirectory(path)) {
        if (path[path.size() - 1] != '/') {
            return (response.createErrorResponse(409, config.root, config.errorPages));
        }

        std::string command = "rm -rf " + path;
        int result = std::system(command.c_str());
        if (result == 0)
            return (response.createResponseFromStatus((204)));
        return (response.createErrorResponse(500, config.root, config.errorPages));
    } else if (remove(path.c_str()) == -1) {
        return (response.createErrorResponse(500, config.root, config.errorPages));
    }

    return (response.createResponseFromStatus(204));
}

int ServerManager::removeClient(int clientSocket) {
    clientSockets.erase(std::find(clientSockets.begin(), clientSockets.end(), clientSocket));
    close(clientSocket);
    return (clientSocket);
}

bool ServerManager::isClient(int clientSocket) const {
    return (std::find(clientSockets.begin(), clientSockets.end(), clientSocket) != clientSockets.end());
}

int ServerManager::getPort() const {
    return (port);
}

in_addr_t ServerManager::getHost() const {
    return (host);
}

int ServerManager::getFd() const {
    return (socketFd);
}

std::vector<Server>::const_iterator ServerManager::findServer(const std::string& host) const {
    size_t pos = host.find(':');
    std::string serverName = (pos == std::string::npos) ? host : host.substr(0, pos);

    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->getName() == serverName) {
            return (it);
        }
    }

    return (servers.begin());
}

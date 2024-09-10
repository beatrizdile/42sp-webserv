#include "ServerManager.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
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
        logger.error() << "Error: " << strerror(errno) << std::endl;
        return (removeClient(clientSocket));
    }

    if (bytesRead == 0) {
        logger.info() << "Client disconnected" << std::endl;
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
            logger.error() << "Error: failed to send response" << std::endl;
            return (removeClient(clientSocket));
        }

        logger.error() << "Error: failed to parse request" << std::endl;
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
        responseString = processRequest((*server).getRoot(), request.getUri(), (*server).getIndex(), (*server).getAutoindex(), (*server).getMethods(), (*server).getClientBodySize(), request.getHeaders());
    } else {
        responseString = processRequest((*location).getRoot(), request.getUri(), (*location).getIndex(), (*location).getAutoindex(), (*location).getMethods(), (*location).getClientBodySize(), request.getHeaders());
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

std::string ServerManager::createPath(const std::string& root, const std::string& uri) {
    if (root.empty()) {
        return (uri);
    }

    if (root[root.size() - 1] == '/') {
        return (root.substr(0, root.size() - 1) + uri);
    }

    return (root + uri);
}

std::string ServerManager::processRequest(const std::string& root, const std::string& uri, const std::string& index, bool isAutoindex, const std::vector<Method>& methods, size_t clientBodySize, const std::map<std::string, std::string>& headers) {
    std::string path = createPath(root, uri);

    if (std::find(methods.begin(), methods.end(), request.getMethod()) == methods.end()) {
        return (response.createResponseFromStatus(405));
    }

    if (request.getBody().size() > clientBodySize) {
        return (response.createResponseFromStatus(413));
    }

    if (request.getMethod() == GET) {
        return (processGetRequest(path, uri, index, isAutoindex));
    } else if (request.getMethod() == POST) {
        return (processPostRequest(path, uri, headers));
    } else if (request.getMethod() == DELETE) {
        return (processDeleteRequest(path));
    }
    return (response.createResponseFromStatus(501));
}

std::string ServerManager::processGetRequest(const std::string& path, const std::string& uri, const std::string& index, bool isAutoindex) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        return (response.createResponseFromStatus(404));
    }
    std::string etag = request.getEtag();

    if (S_ISDIR(fileStat.st_mode)) {
        if (path[path.size() - 1] != '/') {
            return (response.createResponseFromStatus(301));
        } else if (access((path + "/" + index).c_str(), F_OK) != -1) {
            return (response.createFileResponse(path + "/" + index, etag));
        } else if (isAutoindex) {
            return (response.createIndexResponse(path, uri));
        } else {
            return (response.createResponseFromStatus(403));
        }
    } else if (S_ISREG(fileStat.st_mode)) {
        return (response.createFileResponse(path, etag));
    } else {
        return (response.createResponseFromStatus(404));
    }
}

std::string ServerManager::processPostRequest(const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers) {
    if (request.getBody().empty()) {
        return (response.createResponseFromStatus(400));
    }

    std::map<std::string, std::string>::const_iterator it = headers.find(HttpRequest::HEADER_CONTENT_TYPE_KEY);
    const std::string& contentType = (it != headers.end()) ? it->second : "application/octet-stream";
    if (contentType != "text/plain" && contentType != "application/octet-stream") {
        return (response.createResponseFromStatus(415));
    }

    if (access(path.c_str(), F_OK) != -1) {
        return (response.createResponseFromStatus(409));
    }
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        return (response.createResponseFromStatus(500));
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

std::string ServerManager::processDeleteRequest(const std::string& path) {
    if (access(path.c_str(), F_OK) == -1) {
        return (response.createResponseFromStatus(404));
    }
    if (access(path.c_str(), W_OK) == -1) {
        return (response.createResponseFromStatus(403));
    }

    if (isDirectory(path)) {
        if (path[path.size() - 1] != '/') {
            return (response.createResponseFromStatus(409));
        }

        std::string command = "rm -rf " + path;
        int result = std::system(command.c_str());
        if (result == 0)
            return (response.createResponseFromStatus((204)));
        return (response.createResponseFromStatus((500)));
    } else if (remove(path.c_str()) == -1) {
        return (response.createResponseFromStatus(500));
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

#include "Client.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>

const size_t Client::BUFFER_SIZE = 1024;

Client::Client() {}

Client::Client(int fd) : fd(fd), logger(Logger("CLIENT")) {}

Client::~Client() {}

Client::Client(const Client &other) {
	*this = other;
}

Client &Client::operator=(const Client &other) {
	if (this != &other) {
		this->fd = other.fd;
		this->request = other.request;
		this->response = other.response;
		this->responseStr = other.responseStr;
		this->logger = other.logger;
	}
	return *this;
}

int Client::getFd() const{
	return this->fd;
}

int Client::processSendedData(const std::vector<Server>& servers) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = 0;

    bytesRead = recv(fd, buffer, BUFFER_SIZE, 0);
    if (bytesRead == -1) {
        logger.perror("recv");
        return (fd);
    }

    if (bytesRead == 0) {
        logger.info() << "Client disconnected on fd " << fd << std::endl;
        return (fd);
    }

    if (!request.digestRequest(std::string(buffer, bytesRead))) {
        responseStr = response.createResponseFromStatus(400);
        return (0);
    }

    if (request.isComplete()) {
        matchUriAndResponseClient(servers);
    }

    return (0);
}

int Client::sendResponse() {
    if (responseStr.size() == 0) {
        return (0);
    }

    ssize_t bytesToSend = std::min(responseStr.size(), BUFFER_SIZE);
	std::string buffer = responseStr.substr(0, bytesToSend);

	ssize_t bytesSend = send(fd, buffer.c_str(), buffer.size(), 0);
    if (bytesSend == -1) {
        logger.perror("send");
        return (fd);
    }
    if (bytesSend != bytesToSend) {
        logger.error() << "Error: failed to send response" << std::endl;
        return (fd);
    }

    responseStr.erase(0, bytesToSend);
    return (0);
}

std::vector<Server>::const_iterator Client::findServer(const std::vector<Server>& servers, const std::string& host) const {
    size_t pos = host.find(':');
    std::string serverName = (pos == std::string::npos) ? host : host.substr(0, pos);

    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->getName() == serverName) {
            return (it);
        }
    }

    return (servers.begin());
}

void Client::matchUriAndResponseClient(const std::vector<Server>& servers) {
    logger.info() << "Request: " << getMethodString(request.getMethod()) << " " << request.getUri() << " " << request.getVersion() << std::endl;

    // Find server that match with "Host" header
    std::vector<Server>::const_iterator server = findServer(servers, request.getHeaders().at(HttpRequest::HEADER_HOST_KEY));

    // Find location in server that match with URI
    std::vector<Location>::const_iterator location = (*server).matchUri(request.getUri());

    // Process request
    if (location == (*server).getLocations().end()) {
        responseStr = processRequest((*server).getConfig(), request.getUri(), request.getHeaders());
    } else {
        responseStr = processRequest((*location).getConfig(), request.getUri(), request.getHeaders());
    }
    request.clear();
}

std::string Client::processRequest(const t_config& config, const std::string& uri, const std::map<std::string, std::string>& headers) {
    if (std::find(config.methods.begin(), config.methods.end(), request.getMethod()) == config.methods.end()) {
        return (response.createErrorResponse(405, config.root, config.errorPages));
    }

    if (request.getBody().size() > config.clientBodySize) {
        return (response.createErrorResponse(413, config.root, config.errorPages));
    }

    if (config.redirect != "") {
        return (response.createResponseFromLocation(301, config.redirect));
    }

    std::string path = createPath(config.root, uri);
    if (request.getMethod() == GET) {
        return (processGetRequest(config, path, uri));
    } else if (request.getMethod() == POST) {
        return (processPostRequest(config, path, uri, headers));
    } else if (request.getMethod() == DELETE) {
        return (processDeleteRequest(config, path));
    }
    return (response.createErrorResponse(501, config.root, config.errorPages));
}

std::string Client::processGetRequest(const t_config& config, const std::string& path, const std::string& uri) {
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

std::string Client::processPostRequest(const t_config& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers) {
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

std::string Client::processDeleteRequest(const t_config& config, const std::string& path) {
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

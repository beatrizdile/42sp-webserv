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

Client::Client() : fd(0), pipeIn(0), pipeOut(0), logger(Logger("Client")) {}

Client::Client(int fd) : fd(fd), pipeIn(0), pipeOut(0), logger(Logger("Client")) {}

Client::~Client() {}

Client::Client(const Client& other) {
    *this = other;
}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        this->fd = other.fd;
        this->pipeIn = other.pipeIn;
        this->pipeOut = other.pipeOut;
        this->request = other.request;
        this->response = other.response;
        this->responseStr = other.responseStr;
        this->logger = other.logger;
    }
    return *this;
}

int Client::getFd() const {
    return this->fd;
}

bool Client::isFdValid(int fd) const {
    return (fd == this->fd || fd == pipeIn || fd == pipeOut);
}

std::string Client::createCgiProcess(const t_config& config, std::string& execPath, std::string& scriptPath, std::vector<pollfd>& fdsToAdd) {
    if (access(scriptPath.c_str(), F_OK) == -1) {
        return (response.createErrorResponse(404, config.root, config.errorPages));
    }

    int pipeInput[2], pipeOutput[2];
    pipeInput[0] = pipeInput[1] = -1;

    if (pipe(pipeOutput) == -1) {
        return (response.createErrorResponse(500, config.root, config.errorPages));
    }
    if (!request.getBody().empty() && pipe(pipeInput) == -1) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        return (response.createErrorResponse(500, config.root, config.errorPages));
    }

    int pid = fork();
    if (pid == -1) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        if (pipeInput[0] != -1) {
            close(pipeInput[0]);
            close(pipeInput[1]);
        }
        return (response.createErrorResponse(500, config.root, config.errorPages));
    }

    if (pid == 0) {
        if (pipeInput[0] != -1) {
            close(pipeInput[1]);
            dup2(pipeInput[0], STDIN_FILENO);
            close(pipeInput[0]);
        }

        close(pipeOutput[0]);
        dup2(pipeOutput[1], STDOUT_FILENO);
        close(pipeOutput[1]);

        std::string dir = scriptPath.substr(0, scriptPath.find_last_of('/'));
        if (chdir(dir.c_str()) == -1) {
            exit(1);
        }

        setenv("REQUEST_METHOD", getMethodString(request.getMethod()).c_str(), 1);
        setenv("REQUEST_URI", request.getUri().c_str(), 1);
        setenv("SERVER_PROTOCOL", request.getVersion().c_str(), 1);
        setenv("SERVER_SOFTWARE", "webserv", 1);
        setenv("SCRIPT_NAME", scriptPath.c_str(), 1);
        setenv("CONTENT_LENGTH", numberToString(request.getBody().size()).c_str(), 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);

        for (std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); ++it) {
            setenv(("HTTP_" + it->first).c_str(), it->second.c_str(), 1);
        }

        char* argv[] = {(char*)execPath.c_str(), (char*)scriptPath.c_str(), NULL};
        execv(execPath.c_str(), argv);

        exit(1);
    } else {
        if (pipeInput[0] != -1) {
            close(pipeInput[0]);
            pipeIn = pipeInput[1];

            struct pollfd pfdIn;
            pfdIn.fd = pipeIn;
            pfdIn.events = POLLOUT;
            pfdIn.revents = 0;
            fdsToAdd.push_back(pfdIn);
        }

        close(pipeOutput[1]);
        pipeOut = pipeOutput[0];

        struct pollfd pfdOut;
        pfdOut.fd = pipeOut;
        pfdOut.events = POLLIN;
        pfdOut.revents = 0;
        fdsToAdd.push_back(pfdOut);

        return ("");
    }
}

void Client::closeAll() const {
    if (pipeIn != 0) {
        close(pipeIn);
    }
    if (pipeOut != 0) {
        close(pipeOut);
    }
}

int Client::processSendedData(int fdAffected, const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = 0;

    bytesRead = read(fdAffected, buffer, BUFFER_SIZE);
    if (bytesRead == -1) {
        logger.perror("read");
        if (fdAffected != fd) {
            pipeOut = 0;
        }
        return (fdAffected);
    }

    if (bytesRead == 0) {
        logger.info() << "Client disconnected on fd " << fdAffected << std::endl;
        if (fdAffected != fd) {
            pipeOut = 0;
        }
        return (fdAffected);
    }

    if (fdAffected != fd) {
        return (readCgiResponse(std::string(buffer, bytesRead)));
    }

    if (!request.digestRequest(std::string(buffer, bytesRead))) {
        responseStr = response.createResponseFromStatus(400);
        return (0);
    }

    if (request.isComplete()) {
        matchUriAndResponseClient(servers, fdsToAdd);
    }

    return (0);
}

int Client::readCgiResponse(const std::string& bytesReded) {
    logger.info() << "Cgi response: " << bytesReded << std::endl;
    responseStr = response.createResponseFromStatus(204);
    return (0);
}

int Client::sendResponse(int clientSocket) {
    if (clientSocket == fd && responseStr.size() == 0) {
        return (0);
    } else if (clientSocket != fd && request.getBody().size() == 0) {
        pipeIn = 0;
        return (clientSocket);
    }

    std::string buffer = (clientSocket == fd) ? responseStr : request.getBody();
    ssize_t bytesToSend = std::min(buffer.size(), BUFFER_SIZE);
    std::string data = buffer.substr(0, bytesToSend);

    ssize_t bytesSend = write(fd, data.c_str(), data.size());
    if (bytesSend == -1) {
        if (clientSocket != fd) {
            pipeIn = 0;
        }
        logger.perror("write");
        return (clientSocket);
    }
    if (bytesSend != bytesToSend) {
        if (clientSocket != fd) {
            pipeIn = 0;
        }
        logger.error() << "Error: failed to send response" << std::endl;
        return (clientSocket);
    }

    if (clientSocket == fd) {
        responseStr.erase(0, bytesToSend);
    } else {
        request.eraseBody(bytesToSend);
    }
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

void Client::matchUriAndResponseClient(const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd) {
    logger.info() << "Request: " << getMethodString(request.getMethod()) << " " << request.getUri() << " " << request.getVersion() << std::endl;

    // Find server that match with "Host" header
    std::vector<Server>::const_iterator server = findServer(servers, request.getHeaders().at(HttpRequest::HEADER_HOST_KEY));

    // Find location in server that match with URI
    std::vector<Location>::const_iterator location = (*server).matchUri(request.getUri());

    // Process request
    if (location == (*server).getLocations().end()) {
        responseStr = processRequest((*server).getConfig(), request.getUri(), request.getHeaders(), fdsToAdd);
    } else {
        responseStr = processRequest((*location).getConfig(), request.getUri(), request.getHeaders(), fdsToAdd);
    }
    request.clear();
}

static std::string findCgiPath(std::string path, const t_config& config) {
    if (config.cgiPaths.size() == 0) {
        return ("");
    }

    size_t dotPosition = path.find_last_of('.');
    if (dotPosition == std::string::npos) {
        return ("");
    }

    std::string extension = path.substr(dotPosition + 1);
    std::map<std::string, std::string>::const_iterator it = config.cgiPaths.find(extension);
    if (it != config.cgiPaths.end()) {
        return (it->second);
    }

    return ("");
}

std::string Client::processRequest(const t_config& config, const std::string& uri, const std::map<std::string, std::string>& headers, std::vector<pollfd>& fdsToAdd) {
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
    std::string execPath = findCgiPath(path, config);
    if (!execPath.empty()) {
        return (createCgiProcess(config, execPath, path, fdsToAdd));
    } else if (request.getMethod() == GET) {
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
            return (response.createResponseFromLocation(301, uri + "/"));
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

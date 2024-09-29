#include "Client.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

const size_t Client::READ_BUFFER_SIZE = 1024 * 2;          // 2 KB
const size_t Client::WRITE_BUFFER_SIZE = 1024 * 1024 * 1;  // 1 MB
const long long Client::CGI_TIMEOUT_IN_SECONDS = 2;        // 2 seconds

Client::Client() : fd(0), pipeIn(0), pipeOut(0), request(), response(), responseStr(""), cgiOutputStr(""), cgiInputStr(""), cgiPid(0), cgiStarProcessTimestamp(0), cgiConfig(), logger("CLIENT") {}

Client::Client(int fd) : fd(fd), pipeIn(0), pipeOut(0), request(), response(), responseStr(""), cgiOutputStr(""), cgiInputStr(""), cgiPid(0), cgiStarProcessTimestamp(0), cgiConfig(), logger("CLIENT") {}

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
        this->cgiOutputStr = other.cgiOutputStr;
        this->cgiInputStr = other.cgiInputStr;
        this->cgiPid = other.cgiPid;
        this->logger = other.logger;
        this->cgiConfig = other.cgiConfig;
    }
    return *this;
}

int Client::getFd() const {
    return this->fd;
}

int Client::getPipeOut() const {
    return this->pipeOut;
}

bool Client::isFdValid(int fd) const {
    return (fd == this->fd || fd == pipeIn || fd == pipeOut);
}

static long long getTimestampSeconds() {
    std::time_t now = std::time(0);
    return (static_cast<long long>(now));
}

static int setNonBlockingFlag(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return (-1);
    }
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

std::string Client::createCgiProcess(const Configurations& config, std::string& execPath, std::string& scriptPath, std::vector<pollfd>& fdsToAdd) {
    if (access(scriptPath.c_str(), F_OK) == -1) {
        return (response.createErrorResponse(404, config.getRoot(), config.getErrorPages()));
    }

    int pipeInput[2], pipeOutput[2];
    pipeInput[0] = pipeInput[1] = -1;

    if (pipe(pipeOutput) == -1) {
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    }

    if (setNonBlockingFlag(pipeOutput[0]) == -1 || setNonBlockingFlag(pipeOutput[1]) == -1) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    }

    if (!request.getBody().empty() && pipe(pipeInput) == -1) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    }

    if (pipeInput[0] != -1 && (setNonBlockingFlag(pipeInput[0]) == -1 || setNonBlockingFlag(pipeInput[1]) == -1)) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        close(pipeInput[0]);
        close(pipeInput[1]);
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    }

    cgiStarProcessTimestamp = getTimestampSeconds();
    int pid = fork();
    if (pid == -1) {
        close(pipeOutput[0]);
        close(pipeOutput[1]);
        if (pipeInput[0] != -1) {
            close(pipeInput[0]);
            close(pipeInput[1]);
        }
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
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
        std::string cookies;
        for (std::map<std::string, std::string>::const_iterator it = request.getCookies().begin(); it != request.getCookies().end(); ++it) {
            cookies += it->first + "=" + it->second + "; ";
        }
        if (!cookies.empty()) {
            setenv("HTTP_COOKIE", cookies.c_str(), 1);
        }
        setenv("SCRIPT_NAME", scriptPath.c_str(), 1);
        setenv("QUERY_STRING", request.getQueryParameters().c_str(), 1);
        setenv("CONTENT_LENGTH", numberToString(request.getBody().size()).c_str(), 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);

        for (std::map<std::string, std::string>::const_iterator it = request.getHeaders().begin(); it != request.getHeaders().end(); ++it) {
            setenv(("HTTP_" + it->first).c_str(), it->second.c_str(), 1);
        }

        std::string scriptFile = scriptPath.substr(scriptPath.find_last_of('/') + 1);
        char* argv[] = {(char*)execPath.c_str(), (char*)scriptFile.c_str(), NULL};
        execv(execPath.c_str(), argv);

        exit(1);
    } else {
        cgiPid = pid;
        cgiConfig = config;
        cgiInputStr = request.getBody();
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
    char buffer[READ_BUFFER_SIZE];
    ssize_t bytesRead = 0;

    bytesRead = read(fdAffected, buffer, READ_BUFFER_SIZE);
    if (bytesRead == -1) {
        logger.perror("read");
        if (fdAffected != fd) {
            pipeOut = 0;
        }
        return (fdAffected);
    }

    if (bytesRead == 0) {
        if (fdAffected != fd) {
            pipeOut = 0;
            readCgiResponse();
        }
        return (fdAffected);
    }

    if (fdAffected != fd) {
        cgiOutputStr += std::string(buffer, bytesRead);
        return (0);
    }

    if (!request.digestRequest(std::string(buffer, bytesRead))) {
        Configurations config = servers.begin()->getConfig();
        responseStr = response.createErrorResponse(400, config.getRoot(), config.getErrorPages());
        return (0);
    }

    if (request.isComplete()) {
        matchUriAndResponseClient(servers, fdsToAdd);
    }

    return (0);
}

void Client::readCgiResponse() {
    int status;
    waitpid(cgiPid, &status, 0);
    cgiPid = 0;

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        responseStr = response.createErrorResponse(500, cgiConfig.getRoot(), cgiConfig.getErrorPages());
        cgiOutputStr.clear();
        return;
    }

    std::map<std::string, std::string> responseHeaders;
    std::istringstream responseStream(cgiOutputStr);
    std::string line;
    std::vector<std::string> cookies;
    while (std::getline(responseStream, line) && line != "\r" && line != "") {
        size_t pos = line.find(": ");
        if (pos == std::string::npos) {
            responseStr = response.createErrorResponse(500, cgiConfig.getRoot(), cgiConfig.getErrorPages());
            return;
        }
        std::string key = line.substr(0, pos);
        if (HttpRequest::verifyHeaderKey(key)) {
            responseStr = response.createErrorResponse(500, cgiConfig.getRoot(), cgiConfig.getErrorPages());
            return;
        }
        std::string value = line.substr(pos + 2);
        trim(value);
        if (HttpRequest::verifyHeaderValue(value)) {
            responseStr = response.createErrorResponse(500, cgiConfig.getRoot(), cgiConfig.getErrorPages());
            return;
        }
        std::string headerKey = key;
        lowercase(headerKey);
        if (headerKey == "set-cookie") {
            cookies.push_back(value);
        } else {
            responseHeaders[key] = value;
        }
    }

    bool findContentType = false;
    for (std::map<std::string, std::string>::const_iterator it = responseHeaders.begin(); it != responseHeaders.end(); ++it) {
        std::string headerKey = it->first;
        lowercase(headerKey);
        if (headerKey == "content-type") {
            findContentType = true;
            break;
        }
    }
    if (!findContentType) {
        responseStr = response.createErrorResponse(500, cgiConfig.getRoot(), cgiConfig.getErrorPages());
        return;
    }

    std::string body;
    while (std::getline(responseStream, line)) {
        body += line + "\n";
    }

    cgiOutputStr.clear();
    responseStr = response.createCgiResponse(200, body, responseHeaders, cookies);
}

void Client::verifyCgiTimeout(std::vector<int>& fdsToRemove) {
    if (cgiPid == 0) {
        return;
    }

    long long currentTimestamp = getTimestampSeconds();
    if (currentTimestamp - cgiStarProcessTimestamp > CGI_TIMEOUT_IN_SECONDS) {
        kill(cgiPid, SIGKILL);
        cgiPid = 0;
        cgiOutputStr.clear();
        cgiInputStr.clear();
        fdsToRemove.push_back(pipeOut);
        fdsToRemove.push_back(pipeIn);
        pipeOut = 0;
        pipeIn = 0;
        responseStr = response.createErrorResponse(408, cgiConfig.getRoot(), cgiConfig.getErrorPages());
    }
}

int Client::sendResponse(int clientSocket) {
    if (clientSocket == fd && responseStr.size() == 0) {
        return (0);
    } else if (clientSocket != fd && cgiInputStr.size() == 0) {
        pipeIn = 0;
        return (clientSocket);
    }

    std::string buffer = (clientSocket == fd) ? responseStr : cgiInputStr;
    ssize_t bytesToSend = std::min(buffer.size(), WRITE_BUFFER_SIZE);
    std::string data = buffer.substr(0, bytesToSend);

    ssize_t bytesSend = write(clientSocket, data.c_str(), data.size());
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
        cgiInputStr.erase(0, bytesToSend);
    }

    if (clientSocket != fd && cgiInputStr.size() == 0) {
        pipeIn = 0;
        return (clientSocket);
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
    std::string cookies = "Cookies:";
    if (request.getCookies().size() > 0) {
        for (std::map<std::string, std::string>::const_iterator it = request.getCookies().begin(); it != request.getCookies().end(); ++it) {
            cookies += ' ' + it->first + "=" + it->second + ";";
        }
    }
    logger.info() << "Request: " << getMethodString(request.getMethod()) << ' ' << request.getUri() << ' ' << request.getVersion() << ' ' << cookies << std::endl;
    std::vector<Server>::const_iterator server = findServer(servers, request.getHeaders().at(HttpRequest::HEADER_HOST_KEY));
    std::vector<Location>::const_iterator location = (*server).matchUri(request.getUri());
    if (location == (*server).getLocations().end()) {
        responseStr = processRequest((*server).getConfig(), fdsToAdd);
    } else {
        responseStr = processRequest((*location).getConfig(), fdsToAdd);
    }
    request.clear();
}

static std::string findCgiPath(std::string path, const Configurations& config) {
    if (config.getCgiPaths().size() == 0) {
        return ("");
    }

    size_t dotPosition = path.find_last_of('.');
    if (dotPosition == std::string::npos) {
        return ("");
    }

    std::string extension = path.substr(dotPosition + 1);
    std::map<std::string, std::string>::const_iterator it = config.getCgiPaths().find(extension);
    if (it != config.getCgiPaths().end()) {
        return (it->second);
    }

    return ("");
}

std::string Client::processRequest(const Configurations& config, std::vector<pollfd>& fdsToAdd) {
    if (std::find(config.getMethods().begin(), config.getMethods().end(), request.getMethod()) == config.getMethods().end()) {
        return (response.createErrorResponse(405, config.getRoot(), config.getErrorPages()));
    }

    if (request.getBody().size() > config.getClientBodySize()) {
        return (response.createErrorResponse(413, config.getRoot(), config.getErrorPages()));
    }

    if (config.getRedirect() != "") {
        return (response.createResponseFromLocation(301, config.getRedirect()));
    }

    std::string path = createPath(config.getRoot(), request.getUri());
    std::string execPath = findCgiPath(path, config);
    if (!execPath.empty()) {
        return (createCgiProcess(config, execPath, path, fdsToAdd));
    } else if (request.getMethod() == GET) {
        return (processGetRequest(config, path, request.getUri()));
    } else if (request.getMethod() == POST) {
        return (processPostRequest(config, path, request.getUri(), request.getHeaders()));
    } else if (request.getMethod() == DELETE) {
        return (processDeleteRequest(config, path));
    }
    return (response.createErrorResponse(501, config.getRoot(), config.getErrorPages()));
}

std::string Client::processGetRequest(const Configurations& config, const std::string& path, const std::string& uri) {
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        return (response.createErrorResponse(404, config.getRoot(), config.getErrorPages()));
    }
    std::string etag = request.getEtag();

    if (S_ISDIR(fileStat.st_mode)) {
        if (path[path.size() - 1] != '/') {
            return (response.createResponseFromLocation(301, uri + '/'));
        } else if (access((path + '/' + config.getIndex()).c_str(), F_OK) != -1) {
            return (response.createFileResponse(path + '/' + config.getIndex(), etag, config.getRoot(), config.getErrorPages()));
        } else if (config.getIsAutoindex()) {
            return (response.createIndexResponse(path, uri, config.getRoot(), config.getErrorPages()));
        } else {
            return (response.createErrorResponse(403, config.getRoot(), config.getErrorPages()));
        }
    } else if (S_ISREG(fileStat.st_mode)) {
        return (response.createFileResponse(path, etag, config.getRoot(), config.getErrorPages()));
    } else {
        return (response.createErrorResponse(404, config.getRoot(), config.getErrorPages()));
    }
}

std::string Client::processPostRequest(const Configurations& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers) {
    if (request.getBody().empty()) {
        return (response.createErrorResponse(400, config.getRoot(), config.getErrorPages()));
    }

    std::map<std::string, std::string>::const_iterator it = headers.find(HttpRequest::HEADER_CONTENT_TYPE_KEY);
    const std::string& contentType = (it != headers.end()) ? it->second : "application/octet-stream";
    if (contentType != "text/plain" && contentType != "application/octet-stream") {
        return (response.createErrorResponse(415, config.getRoot(), config.getErrorPages()));
    }

    if (access(path.c_str(), F_OK) != -1) {
        return (response.createErrorResponse(409, config.getRoot(), config.getErrorPages()));
    }
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
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

std::string Client::processDeleteRequest(const Configurations& config, const std::string& path) {
    if (access(path.c_str(), F_OK) == -1) {
        return (response.createErrorResponse(404, config.getRoot(), config.getErrorPages()));
    }
    if (access(path.c_str(), W_OK) == -1) {
        return (response.createErrorResponse(403, config.getRoot(), config.getErrorPages()));
    }

    if (isDirectory(path)) {
        if (path[path.size() - 1] != '/') {
            return (response.createErrorResponse(409, config.getRoot(), config.getErrorPages()));
        }

        std::string command = "rm -rf " + path;
        int result = std::system(command.c_str());
        if (result == 0)
            return (response.createResponseFromStatus(204));
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    } else if (remove(path.c_str()) == -1) {
        return (response.createErrorResponse(500, config.getRoot(), config.getErrorPages()));
    }

    return (response.createResponseFromStatus(204));
}

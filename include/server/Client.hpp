#pragma once

#include <poll.h>

#include <string>

#include "Configurations.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "Server.hpp"

class Client {
   public:
    static const size_t READ_BUFFER_SIZE;
    static const size_t WRITE_BUFFER_SIZE;
    static const long long CGI_TIMEOUT_IN_SECONDS;

    Client();
    ~Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);

    int getFd() const;
    int getPipeOut() const;
    bool isFdValid(int fd) const;
    int processSendedData(int fdAffected, const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    int sendResponse(int clientSocket);
    void closeAll() const;
    void readCgiResponse();
    void verifyCgiTimeout(std::vector<int>& fdsToRemove);

   private:
    int fd;
    int pipeIn;
    int pipeOut;
    HttpRequest request;
    HttpResponse response;
    std::string responseStr;
    std::string cgiOutputStr;
    std::string cgiInputStr;
    int cgiPid;
    long long cgiStarProcessTimestamp;
    Configurations cgiConfig;
    Logger logger;

    std::string createCgiProcess(const Configurations& config, std::string& execPath, std::string& scriptPath, std::vector<pollfd>& fdsToAdd);
    void matchUriAndResponseClient(const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    std::string processRequest(const Configurations& config, std::vector<pollfd>& fdsToAdd);
    std::string processGetRequest(const Configurations& config, const std::string& path, const std::string& uri);
    std::string processPostRequest(const Configurations& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers);
    std::string processDeleteRequest(const Configurations& config, const std::string& path);
    std::vector<Server>::const_iterator findServer(const std::vector<Server>& servers, const std::string& host) const;
};

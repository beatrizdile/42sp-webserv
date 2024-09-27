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
    static const size_t BUFFER_SIZE;

    Client();
    ~Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);

    int getFd() const;
    bool isFdValid(int fd) const;
    int processSendedData(int fdAffected, const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    int sendResponse(int clientSocket);
    void closeAll() const;

   private:
    int fd;
    int pipeIn;
    int pipeOut;
    HttpRequest request;
    HttpResponse response;
    std::string responseStr;
    std::string cgiOutputStr;
    std::string cgiInputStr;
    Configurations cgiConfig;
    int cgiPid;
    Logger logger;

    void readCgiResponse();
    std::string createCgiProcess(const Configurations& config, std::string& execPath, std::string& scriptPath, std::vector<pollfd>& fdsToAdd);
    void matchUriAndResponseClient(const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    std::string processRequest(const Configurations& config, std::vector<pollfd>& fdsToAdd);
    std::string processGetRequest(const Configurations& config, const std::string& path, const std::string& uri);
    std::string processPostRequest(const Configurations& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers);
    std::string processDeleteRequest(const Configurations& config, const std::string& path);
    std::vector<Server>::const_iterator findServer(const std::vector<Server>& servers, const std::string& host) const;
};

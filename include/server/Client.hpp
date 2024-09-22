#pragma once

#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "Logger.hpp"

class Client {
	public:
		static const size_t BUFFER_SIZE;

		Client();
		~Client();
		Client(int fd);
		Client(const Client &other);
		Client &operator=(const Client &other);
		
		int getFd() const;
		int processSendedData(const std::vector<Server>& servers);
		int sendResponse();

	private:
		int fd;
		HttpRequest request;
		HttpResponse response;
		std::string responseStr;
		Logger logger;

		void matchUriAndResponseClient(const std::vector<Server>& servers);
		std::string processRequest(const t_config& config, const std::string& uri, const std::map<std::string, std::string>& headers);
		std::string processGetRequest(const t_config& config, const std::string& path, const std::string& uri);
		std::string processPostRequest(const t_config& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers);
		std::string processDeleteRequest(const t_config& config, const std::string& path);
		std::vector<Server>::const_iterator findServer(const std::vector<Server>& servers, const std::string& host) const;
};

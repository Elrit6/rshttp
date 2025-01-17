#pragma once

#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <winsock2.h>

namespace rshttp {

	class Server {
	private:
		uint16_t port;
		WSADATA wsaData;
		SOCKET svrSock;
		std::map<std::string, std::function<void(SOCKET clientSocket)>> routes;

	public:
		Server(const uint16_t port);
		std::string getFile(const std::string& filePath, const std::map<std::string, std::string>& variables);
		void returnContent(SOCKET cliSock, const std::string& content, const std::string& mime);
		void route(const std::string& routePath, const std::function<void(SOCKET)>& func);
		std::string getSubPage(const std::string& req);
		void handleClient(SOCKET cliSock);
		void run();
	};

}
#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <inttypes.h>
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

	public:
		class Req {
		public:
			std::string method;
			std::string uri;
			std::map<std::string, std::string> params;
			std::map<std::string, std::string> headers;
			Req(const std::string& request);
			inline std::string getMethod() const { return method; };
			inline std::string getUri() const {return uri; };
			inline std::map<std::string, std::string> getParams() const { return params; };
			inline std::map<std::string, std::string> getHeaders() const { return headers; };
		};

		class Res {
		private:
			SOCKET cliSock;
		public:
			uint16_t status = 200;
			std::map<std::string, std::string> headers;
			void returnContent(const std::string& content, const std::string& mime);
			Res(SOCKET cliSock);
		};

		Server(const uint16_t port);
		void route(const std::string& routePath, const std::function<void(SOCKET, Req, Res)>& func);
		std::string getFile(const std::string& filePath, const std::map<std::string, std::string>& variables);
		void run();
		void handleClient(SOCKET cliSock);

	private:
		std::map<std::string, std::function<void(SOCKET clientSocket, Req, Res)>> routes;
	};

}
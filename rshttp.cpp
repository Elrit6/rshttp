#include "rshttp.hpp"

namespace rshttp {

	Server::Req::Req(const std::string& request) {
		uint64_t i = 0;
		while (i < request.size() && request[i] != ' ') {
			method += request[i];
			i++;
		}
		i++;
		while (i < request.size() && request[i] != ' ' && request[i] != '?') {
			uri += request[i];
			i++;
		}
		if (i < request.size() && request[i] == '?') {
			i++;
			std::string key, val;
			bool readingKey = true;
			while (i < request.size() && request[i] != ' ') {
				if (request[i] == '=') {
					readingKey = false;
				} else if (request[i] == '&') {
					params[key] = val;
					key.clear();
					val.clear();
					readingKey = true;
				} else {
					if (readingKey) {
						key += request[i];
					} else {
						val += request[i];
					}
				}
				i++;
			}
			if (!key.empty()) {
				params[key] = val;
			}
		}
	}

	/*
	void Server::Req::info() {
		std::cout 
			<< std::endl
			<< "method: " << method << std::endl
			<< "uri: " << uri << std::endl
			<< "params: " << std::endl;
		for (const std::pair<std::string, std::string> param : params) {
			std::cout << "  " << param.first << ": " << param.second << std::endl;
		}		
	}*/

	Server::Res::Res(SOCKET cliSock) : cliSock(cliSock) {};

	void Server::Res::returnContent(const std::string& content, const std::string& mime) {
		std::stringstream response;
		response << "HTTP/1.1 " << status << " OK\r\n";
		response << "Content-Type: " << mime << "; charset=UTF-8\r\n";
		response << "Content-Length: " << content.size() << "\r\n";
		response << "\r\n";
		response << content;
		std::string responseStr = response.str();
		send(cliSock, responseStr.c_str(), responseStr.length(), 0);
		closesocket(cliSock);		
	}

	Server::Server(const uint16_t port = 8080) : port(port) {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			throw std::runtime_error("Winsock initialization failed.");
		}
		svrSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (svrSock == INVALID_SOCKET) {
			WSACleanup();
			throw std::runtime_error("Socket creation failed.");
		}

		sockaddr_in svrAddress;
		svrAddress.sin_family = AF_INET;
		svrAddress.sin_addr.s_addr = INADDR_ANY;
		svrAddress.sin_port = htons(port);

		if (bind(svrSock, (sockaddr*)&svrAddress, sizeof(svrAddress)) == SOCKET_ERROR) {
			closesocket(svrSock);
			WSACleanup();
			throw std::runtime_error("Binding failed.");
		}
	}	

	void Server::route(const std::string& routePath, const std::function<void(SOCKET, Req, Res)>& func) {
		routes[routePath] = func;
	}

	std::string Server::getFile(const std::string& filePath, const std::map<std::string, std::string>& variables) {
		std::ifstream file(filePath);
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();

		if (!variables.empty()) {
			for (const std::pair<std::string, std::string>& var : variables) {
				uint64_t varPos = content.find(var.first);
				while (varPos != std::string::npos) {
					content.replace(varPos, var.first.length(), var.second);
					varPos = content.find(var.first, varPos + var.second.length()); 
				}
			}			
		}

		return content;
	}

	void Server::run() {
		if (listen(svrSock, SOMAXCONN) == SOCKET_ERROR) {
			closesocket(svrSock);
			WSACleanup();
			throw std::runtime_error("Listening failed");
		}
		std::cout << "Server listening on " << std::to_string(port) << "." << std::endl;
		while (true) {
			sockaddr_in cliAddr;
			int cliAddrSize = sizeof(cliAddr);
			SOCKET cliSock = accept(svrSock, (sockaddr*)&cliAddr, &cliAddrSize);
			if (cliSock == INVALID_SOCKET) {
				std::cerr << "Accepting a connection failed: " << WSAGetLastError() << std::endl;
				continue;
			}
			std::thread handleClientTh(handleClient, this, cliSock);
			handleClientTh.detach();
		}
	}

	void Server::handleClient(SOCKET cliSock) {
		char buffer[1024];
		uint16_t bytesReceived;

		bytesReceived = recv(cliSock, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';

			Req req(buffer);
			Res res(cliSock);

			const std::string uri = req.getUri();
			if (routes.find(uri) != routes.end()) {
				routes[uri](cliSock, req, res);
			} else {
				res.status = 404;
				res.returnContent("404 not found", "text/plain");
			}
		}
	}

}
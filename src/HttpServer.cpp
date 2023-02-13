#include "HttpServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <future>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <regex>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#pragma comment(lib, "Ws2_32.lib")

#define REQ_BUFLEN 512

const std::string ERROR_404_RES =
	"HTTP/1.1 404\n"
	"content-type: text/html\n"
	"\n"
	"<h1>Error 404</h1>";

static void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

static void HandleReq(HttpServer* server, uint64_t client);
static std::string getStaticFile(const std::string& path);

int InitWSA() {
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

int HttpServer::Start(int port) {
	m_port = port;

	addrinfo* result = NULL;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	m_socket = INVALID_SOCKET;
	m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_socket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	iResult = bind(m_socket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		WSACleanup();
		return 1;
	}

	if (listen(m_socket, SOMAXCONN)) {
		std::cout << "listen() failed: " << WSAGetLastError() << std::endl;
		closesocket(m_socket);
		WSACleanup();
		return 1;
	}

	std::cout << "Listening at port: " << port << std::endl;

	while (true) {
		uint64_t client = INVALID_SOCKET;
		client = accept(m_socket, NULL, NULL);
		if (client == INVALID_SOCKET) {
			std::cout << "accept() failed: " << WSAGetLastError() << std::endl;
			closesocket(client);
			continue;
		}

		std::async(std::launch::async, HandleReq, this, client);
		//HandleReq(this, client);
	}

	WSACleanup();

	return 0;
}

void HttpServer::get(const std::string& route, RouteFunc func) {
	m_routeFuncs["GET " + route] = func;
}

RouteFunc HttpServer::getRouteFunc(const std::string& route) {
	return m_routeFuncs[route];
}

static void HandleReq(HttpServer* server, uint64_t client) {
	char recvbuf[REQ_BUFLEN];
	int recvbuflen = REQ_BUFLEN;

	int iResult = recv(client, recvbuf, recvbuflen, 0);
	if (iResult < 0) {
		std::cout << "recv() failed" << WSAGetLastError() << std::endl;
		closesocket(client);
		return;
	}

	std::unordered_map<std::string, std::string> reqHeaders;

	bool isMethodRouteLine = true;
	std::string method, route;
	std::string methodRoute;

	std::stringstream reqHeadersSS(recvbuf);
	for (std::string headerLine; std::getline(reqHeadersSS, headerLine);) {
		std::stringstream headerLineSS(headerLine);

		if (isMethodRouteLine) {
			headerLineSS >> method;
			headerLineSS >> route;
			methodRoute = method + " " + route;
			isMethodRouteLine = false;
			continue;
		}

		std::string headerName;
		std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);
		std::getline(headerLineSS, headerName, ':');

		std::string headerValue;
		std::getline(headerLineSS, headerValue);
		ltrim(headerValue);

		reqHeaders[headerName] = headerValue;
	}

	std::string response;
	if (route.rfind("/static/", 0) == 0) {
		response = getStaticFile(route);
	} else {
		RouteFunc responseFunc = server->getRouteFunc(methodRoute);
		response = responseFunc ? responseFunc(reqHeaders) : ERROR_404_RES;
	}

	iResult = send(client, response.c_str(), response.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send() failed: " << WSAGetLastError() << std::endl;
		closesocket(client);
		return;
	}

	iResult = shutdown(client, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown() failed: " << WSAGetLastError() << std::endl;
		closesocket(client);
		return;
	}
}

static std::string getStaticFile(const std::string& path) {
	std::stringstream pathSS(path);
	std::string fileExt;

	while (std::getline(pathSS, fileExt, '.'));

	bool isBinary = false;
	std::string contentType;

	if (fileExt == "jpg" || fileExt == "jpeg") {
		isBinary = true;
		contentType = "image/jpg";
;	} else if (fileExt == "png") {
		isBinary = true;
		contentType = "image/png";
	} else if (fileExt == "gif") {
		isBinary = true;
		contentType = "image/gif";
	} else if (fileExt == "css") {
		isBinary = false;
		contentType = "text/css";
	} else if (fileExt == "js") {
		isBinary = false;
		contentType = "text/javascript";
	} else {
		contentType = "text/plain";
	}

	std::ifstream file;

	if (isBinary) file.open("." + path, std::ios::binary);
	else file.open("." + path);

	if (!file.is_open()) return ERROR_404_RES;

	std::string fileContent;

	for (std::string line; std::getline(file, line);) {
		fileContent.append(line + "\n");
	}
	
	return "HTTP/1.1 200\ncontent-type: " + contentType + "\n\n" + fileContent;
}

std::string renderHtmlFile(const std::string& htmlFilePath, const rapidjson::Document& data) {
	std::ifstream file;
	file.open("templates/" + htmlFilePath);

	if (!file.is_open()) return ERROR_404_RES;

	std::string html = "HTTP/1.1 200\ncontent-type: text/html\n\n";
	
	int lineNum = 0;
	for (std::string line; std::getline(file, line);) {
		lineNum++;
		if (lineNum == 2 && !data.IsNull()) {
			std::string searchString = "data-cppbekend=\"";
			size_t pos = line.find(searchString, 6);

			if (pos != std::string::npos) {
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				data.Accept(writer);

				std::string string = std::regex_replace(buffer.GetString(), std::regex("\""), "&quot;");
				//std::string string = buffer.GetString();

				line.insert(pos + searchString.size(), string);
			}
		}
		html.append(line + "\n");
	}

	return html;
}

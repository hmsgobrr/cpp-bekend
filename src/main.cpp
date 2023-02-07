#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT	"8080"
#define BUFLEN	512

void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

int main() {
	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}

	addrinfo* result = NULL;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN)) {
		std::cout << "listen() failed: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Listening at port: " << PORT << std::endl;

	SOCKET ClientSocket = INVALID_SOCKET;

	char recvbuf[BUFLEN];
	int recvbuflen = BUFLEN;
	int iSendResult;

	while (true) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "accept() failed: " << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		std::unordered_map<std::string, std::string> reqHeaders;

		bool isMethodPathLine = true;
		std::string method;
		std::string path;

		std::stringstream reqHeadersSS(recvbuf);
		for (std::string headerLine; std::getline(reqHeadersSS, headerLine);) {
			std::stringstream headerLineSS(headerLine);

			if (isMethodPathLine) {
				headerLineSS >> method;
				headerLineSS >> path;
				isMethodPathLine = false;
				continue;
			}

			std::string headerName;
			std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);
			std::getline(headerLineSS, headerName, ':');

			std::string headerValue;
			std::getline(headerLineSS, headerValue);
			ltrim(headerValue);

			reqHeaders[headerName] = headerValue;

			std::cout << headerName << ": " << reqHeaders[headerName] << "\n";
			//std::cout << headerLine << std::endl;
		}

		const char httpHeader[] =
			"HTTP/1.1 200\n"
			"Content-Type: text/html\n"
			"\n"
			"<h1>Wenomechainsama</h1>";
		iSendResult = send(ClientSocket, httpHeader, sizeof(httpHeader), 0);
		if (iSendResult == SOCKET_ERROR) {
			std::cout << "send() failed: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			std::cout << "shutdown() failed: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		closesocket(ClientSocket);
	}

	WSACleanup();

	return 0;
}

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT	"8080"
#define BUFLEN	512

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

		// Get the method and route from first 2 words of the header
		char* reqHeaderToken = strtok(recvbuf, " ");
		char* method = reqHeaderToken;
		reqHeaderToken = strtok(NULL, " ");
		char* route = reqHeaderToken;

		std::cout << "Request received:\n\tMethod:\t" << method << "\n\tRoute:\t" << route << std::endl;

		char httpHeader[] =
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

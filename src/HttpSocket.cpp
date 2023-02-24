#include "HttpSocket.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <future>

#define REQ_BUFLEN 512

int HttpSocket::Listen(int port, std::function<std::string(std::string)>  requestCallback) {
  m_port = port;

	int iResult;

	WSADATA wsaData;

#ifdef _WIN32
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartip failed wit error: " << iResult << std::endl;
		return 1;
	}
#endif

	addrinfo* result = NULL;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
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

		std::async(std::launch::async, [&requestCallback](uint64_t client) {
      char recvbuf[REQ_BUFLEN];
      int recvbuflen = REQ_BUFLEN;

      int iResult = recv(client, recvbuf, recvbuflen, 0);
      if (iResult < 0) {
        std::cout << "recv() failed" << WSAGetLastError() << std::endl;
        closesocket(client);
        return;
      }

      std::string response = requestCallback(recvbuf);

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
    }, client);
		//HandleReq(this, client);
	}

	WSACleanup();

	return 0;
}

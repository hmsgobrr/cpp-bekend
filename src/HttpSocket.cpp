#include "HttpSocket.h"

#include <iostream>
#include <future>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define GETERR() WSAGetLastError()
	#define WSACLEANUP() WSACleanup()
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <cstring>
	#define GETERR() strerror(errno)
	#define WSACLEANUP()
#endif

#define REQ_BUFLEN 512

int HttpSocket::Listen(int port, std::function<std::string(std::string)> requestCallback) {
  m_port = port;

	int iResult;


#ifdef _WIN32
	WSADATA wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartip failed wit error: " << iResult << std::endl;
		return 1;
	}
#endif

	// addrinfo* result = NULL;
	// addrinfo hints;

	// ZeroMemory(&hints, sizeof(hints));
	// hints.ai_family = AF_INET;
	// hints.ai_socktype = SOCK_STREAM;
	// hints.ai_protocol = IPPROTO_TCP;
	// hints.ai_flags = AI_PASSIVE;

	// iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	// if (iResult != 0) {
	// 	std::cout << "getaddrinfo failed: " << iResult << std::endl;
	// 	WSACLEANUP();
	// 	return 1;
	// }

	m_socket = -1;
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == -1) {
		std::cout << "Error at socket(): " << GETERR() << std::endl;
		WSACLEANUP();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	iResult = bind(m_socket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind() failed: " << GETERR() << std::endl;
		closesocket(m_socket);
		WSACLEANUP();
		return 1;
	}

	if (listen(m_socket, SOMAXCONN)) {
		std::cout << "listen() failed: " << GETERR() << std::endl;
		closesocket(m_socket);
		WSACLEANUP();
		return 1;
	}

	std::cout << "Listening at port: " << port << std::endl;

	while (true) {
		uint64_t client = -1;
		client = accept(m_socket, NULL, NULL);
		if (client == -1) {
			std::cout << "accept() failed: " << GETERR() << std::endl;
			closesocket(client);
			continue;
		}

		std::async(std::launch::async, [&requestCallback](uint64_t client) {
      char recvbuf[REQ_BUFLEN];
      int recvbuflen = REQ_BUFLEN;

      int iResult = recv(client, recvbuf, recvbuflen, 0);
      if (iResult < 0) {
        std::cout << "recv() failed" << GETERR() << std::endl;
        closesocket(client);
        return;
      }

      std::string response = requestCallback(recvbuf);

			iResult = send(client, response.c_str(), response.size(), 0);
			if (iResult == SOCKET_ERROR) {
				std::cout << "send() failed: " << GETERR() << std::endl;
				closesocket(client);
				return;
			}

			iResult = shutdown(client, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				std::cout << "shutdown() failed: " << GETERR() << std::endl;
				closesocket(client);
				return;
			}
    }, client);
		//HandleReq(this, client);
	}

	WSACLEANUP();

	return 0;
}

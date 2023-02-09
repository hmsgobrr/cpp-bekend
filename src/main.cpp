#include "HttpServer.h"

#include <iostream>

#define PORT 8080

std::string d(const HeadersUMap&) {
	std::string response =	"HTTP/1.1 200\n"
							"content-type: text/html\n"
							"\n"
							"<h1>nekalakiniwahapapaniwewinatin</h1>"
							"<img src=\"static/maxwell.jpg\" />";
	std::cout << "asd" << "\n";
	return response;
}

int main() {
	int iResult = InitWSA();
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	HttpServer server;

	server.get("/", d);

	server.Start(PORT);
}

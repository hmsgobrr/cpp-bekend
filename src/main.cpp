#include "HttpServer.h"

#include <iostream>

#define PORT 8080

std::string d(const HeadersUMap&) {
	rapidjson::Document data;
	data.SetObject();

	const char* myname = "jeff";
	data.AddMember("username", rapidjson::Value().SetString(rapidjson::StringRef(myname, 4)), data.GetAllocator());
	
	return renderHtmlFile("index.html", data);
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

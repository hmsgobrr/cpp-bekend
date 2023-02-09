#pragma once

#include <unordered_map>
#include <string>

using HeadersUMap = std::unordered_map<std::string, std::string>;
typedef std::string (*RouteFunc)(const HeadersUMap&);

class HttpServer {
public:
	int Start(int port);

	void get(const std::string& route, RouteFunc func);

	RouteFunc getRouteFunc(const std::string& route);
private:
	int m_port = 5000;
	uint64_t m_socket = 0;
	std::unordered_map<std::string, RouteFunc> m_routeFuncs;
};

int InitWSA();

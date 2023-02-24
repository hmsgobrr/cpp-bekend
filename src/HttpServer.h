#pragma once

#include <unordered_map>
#include <string>

#include "rapidjson/document.h"

#include "HttpSocket.h"

using HeadersUMap = std::unordered_map<std::string, std::string>;
typedef std::string (*RouteFunc)(const HeadersUMap&);

class HttpServer {
public:
	int Listen(int port);

	void route(const std::string& route, RouteFunc func);
private:
	HttpSocket m_socket;
	std::unordered_map<std::string, RouteFunc> m_routeFuncs;
};

std::string renderHtmlFile(const std::string& htmlFilePath, const rapidjson::Document& data);

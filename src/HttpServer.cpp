#include "HttpServer.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

static std::string getStaticFile(const std::string& path);

int HttpServer::Listen(int port) {
	return m_socket.Listen(port, [this](std::string requestText) {
		std::unordered_map<std::string, std::string> reqHeaders;

		bool isMethodRouteLine = true;
		std::string method, route;
		std::string methodRoute;

		std::stringstream reqHeadersSS(requestText);
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
			RouteFunc responseFunc = m_routeFuncs[methodRoute];
			response = responseFunc ? responseFunc(reqHeaders) : ERROR_404_RES;
		}

		return response;
	});
}

void HttpServer::route(const std::string& route, RouteFunc func) {
	m_routeFuncs[route] = func;
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
	} else if (fileExt == "png") {
		isBinary = true;
		contentType = "image/png";
	} else if (fileExt == "gif") {
		isBinary = true;
		contentType = "image/gif";
	} else if (fileExt == "mp3") {
		isBinary = true;
		contentType = "audio/mpeg";
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

	if (isBinary) file.open("." + path, std::ios::binary | std::ios::in);
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

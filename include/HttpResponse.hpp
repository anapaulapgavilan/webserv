#pragma once

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../include/WebServ.hpp"


struct ResponseStatus {
    int code;
    std::string message;

	ResponseStatus();
	ResponseStatus(int code);
};

struct ResponseHeaders {
	std::string content_type;
	std::string content_length;
	std::string allow;
	std::string connection;
	std::string location;
};



class HttpResponse
{
private:
	Request *_request;
	ResponseStatus _status_line;
	ResponseHeaders _headers;
	std::string _body;
	
	HttpResponse();
	HttpResponse(const HttpResponse &other);
	HttpResponse &operator=(const HttpResponse &other);
	
public:
	static const std::string CRLF;
	static const std::string version;

	HttpResponse(Request *request);
	HttpResponse(int errorCode);
	HttpResponse(int errorCode, const std::string &errorpage_or_location);
	~HttpResponse();

	std::string getStatusLine() const ;
	std::string getHeaders() const ;
	std::string getBody() const ;
	std::string toString() const ;
	std::string getResponse() const;

	void reset_all();
	void generate_webindex(const Request& request);
	void generate_autoindex(const Request& request);
	void set_redirect_response(int code, const std::string& location);
	void set_empty_response_alive(int code);
	void set_empty_response_close(int code);
	void set_allow_methods(const std::string& methods);

	void handle_GET();
	void handle_POST();

	

};

#endif
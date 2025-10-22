#ifndef CGI_HPP
#define CGI_HPP

#include "WebServ.hpp"

class Cgi
{
	private:
		std::string _scriptPath;
		std::string _method;
		std::string _queryString;
		std::map<std::string, std::string> _envVariables;

		void setEnvVariables(const Request& req);
		std::string executeCgi(const Request& req);

	public:
		Cgi(const std::string& scriptPath);
		~Cgi();

		std::string run(const Request& req);
};
#endif
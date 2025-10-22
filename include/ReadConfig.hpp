#ifndef READCONFIG_HPP
#define READCONFIG_HPP

#include "../include/WebServ.hpp"

#define READ_CONFIG_ERR "Error: ReadConfig Error"
#define INVALID_FILE_ERR "Error: File is Invalid"
#define ACCESS_FILE_ERR "Error: File is not Accessible"
#define EMPTY_FILE_ERR "Error: File is Empty"
#define SIZE_FILE_ERR "Error: Size"
#define FIND_FILE_ERR "Error: Did Not Find Server"
#define SCOPE_FILE_ERR "Error: Error with Scope"
#define SERVER_SCOPE_ERR "Error: Wrong Character out of Server Scope{}"
#define SERVER_VALIDATION_ERR "Error: Failed Server Validation"
#define PORT_ERR "Error: Port is Duplicated or Not Found"
#define HOST_ERR "Error: Host is Duplicated"
#define ROOT_ERR "Error: Root is Duplicated"
#define CLIENT_ERR "Error: Client_max_body_size is Duplicated"
#define SERVER_NAME_ERR "Error: Server Name is Duplicated"
#define INDEX_ERR "Error: Index is Duplicated"
#define AUTOINDEX_ERR "Error: Autoindex error"
#define DIRECTIVE_ERR "Error: Unsupported Directive"
#define INDEX_CONFIG_ERR "Error: Index from Config File Unreadable"
#define LOCATION_DUP_ERR "Error: Location is Duplicated"
#define ERROR_PAGE_ERR "Error: Incorrect Error Page or Number of Error"
#define SERVER_ERR "Error: Failed Server Validation"

class ServerUnit;

class ReadConfig {
	private:
		std::vector<ServerUnit>	_servers;
		std::vector<std::string>	_server_config;
		size_t						_nb_server;

	public:

		ReadConfig();
		~ReadConfig();

		int                         createServerGroup(const std::string &config_file);
		void                        splitServers(std::string &content);
		void                        removeComments(std::string &content);
		void                        removeWhiteSpace(std::string &content);
		size_t                      findStartServer(size_t start, std::string &content);
		size_t                      findEndServer(size_t start, std::string &content);
		void                        createServer(std::string &config, ServerUnit &server);
		void                        checkServers();
		std::vector<ServerUnit>			getServers();

		public:
		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = READ_CONFIG_ERR + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif
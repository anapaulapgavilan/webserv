#ifndef SERVERSETUP_HPP
#define SERVERSETUP_HPP

#include "./WebServ.hpp"

#define SYNTAX_ERR_HOST "Syntax Error: host"
#define SYNTAX_ERR_ROOT "Syntax Error: root"
#define SYNTAX_ERR_PORT "Syntax Error: port"
#define SYNTAX_ERR_CLIENT_MAX_SIZE "Syntax Error: client_max_body_size"
#define SYNTAX_ERR_AUTOINDEX "Syntax Error: autoindex"
#define SYNTAX_ERR_INDEX "Syntax Error: index"
#define SYNTAX_ERR_CGI_EXT "Syntax Error: cgi extension must start with . "
#define SYNTAX_ERR_CGI_PATH "Syntax Error: cgi path must start with ./"
#define TOKEN_ERR "Error: Invalid Token"
#define PAGE_ERR_INIT "Error: Page Initialization Failed"
#define PAGE_ERR_CODE "Error: Code is Invalid"
#define INCORRECT_ERR_CODE "Incorrect Error Code: "
#define INCORRECT_ERR_PATH "Incorrect Path for Error File: "
#define ROOT_DUP_ERR "Error: Root of Location is Duplicated"
#define METHODS_DUP_ERR "Error: Allow Methods of Location is Duplicated"
#define AUTOINDEX_DUP_ERR "Error: AutoIndex of Location is Duplicated"
#define INDEX_DUP_ERR "Error: Index of Location is Duplicated"
#define RETURN_DUP_ERR "Error: Return of Location is Duplicated"
#define RETURN_ERR_CGI "Error: Return Not Allowed for CGI"
#define ALIAS_DUP_ERR "Error: Alias of Location is Duplicated"
#define ALIAS_ERR_CGI "Error: Alias Not Allowed for CGI"
#define CMBS_DUP_ERR "Error: Client Max Body Size of Location is Duplicated"
#define INVLAID_CGI_ERR "Error: cgi_path is Invalid"
#define CGI_EXT_DUP_ERR "Error: cgi is Duplicated"
#define CGI_ERR_VALIDATION "Failed CGI Validation"
#define LOCATION_ERR_VALIDATION "Failed Location Validation"
#define REDIRECTION_ERR_VALIDATION "Failed Redirection Validation"
#define ALIAS_ERR_VALIDATION "Failed Alias Validation"
#define INDEX_ERR_VALIDATION "Failed Index Validation"
#define ERR_PAGE_ERR "Error: Error Page Does Not Exist"
#define LOCATION_ERR "Error: Path to Location Not Found"
#define ERROR_SERV_PARSER "Error in our ServerSetUp: "
#define SOCKET_ERR "Error: socket() failed: "
#define SET_SOCKET_ERR "Error: setsockopt() failed: "
#define BIND_ERR "Error: bind() failed: "


#define UNDEFINED "Undefined"
#define UNACCESSIBLE "Unaccessible"

#define ROOT "root"
#define ALLOW_METHODS "allow_methods"
#define METHODS "methods"
#define AUTOINDEX "autoindex"
#define INDEX "index"
#define RETURN "return"
#define ALIAS "alias"
#define CGI "cgi" // directive: cgi extension ./cgi_path;
#define CGI_BIN_PATH "/cgi-bin"
#define CMBS "client_max_body_size"

class Location;

enum e_err_validation {
	ER_VAL_CGI = 1,
	ER_VAL_LOCATION = 2,
	ER_VAL_REDIRECT = 3,
	ER_VAL_ALIAS = 4,
	ER_VAL_INDEX = 5,
	ER_VAL_CGI_PATH_NONEXE = 6,

};

class ServerUnit
{
	private:
		uint16_t						_port;
		in_addr_t						_host;
		std::string						_server_name;
		std::string						_root;
		unsigned long					_client_max_body_size;
		std::string						_index;
		bool							_autoindex;
		std::map<short, std::string>	_error_list;
		std::vector<Location> 			_locations;
		int     						_listen_fd;
		struct sockaddr_in 				_server_address;

	public:
		ServerUnit();
		~ServerUnit(); 
		ServerUnit(const ServerUnit &other);
		ServerUnit &operator=(const ServerUnit & rhs);

		void                                    		setServerName(std::string server_name);
		void                                    		setHost(std::string parameter);
		void                                    		setRoot(std::string root);
		void                                    		setPort(std::string token);
		void                                    		setClientMaxBodySize(std::string token);
		void                                    		setErrorPages(std::vector<std::string> &token);
		void                                    		setIndex(std::string index);
		void                                    		setLocation(std::string nameLocation, std::vector<std::string> token);
		void                                    		setAutoindex(std::string autoindex);

		bool                                    		isValidHost(std::string host) const;
		bool                                    		isValidErrorPages();
		int                                     		isValidLocation(Location &location) const;

		const std::string                       		&getServerName();
		const uint16_t  								&getPort() const;
		const in_addr_t									&getHost() const;
		const size_t                            		&getClientMaxBodySize(); 
		const std::string                       		&getRoot(); 
		const std::vector<Location>       				&getLocations();
		const std::map<short, std::string>      		&getErrorPages();
		const std::string                       		&getIndexFiles();
		const std::string								&getIndex();
		const bool                              		&getAutoindex(); 
		const std::string                       		&getPathErrorPage(short key); 
		const std::vector<Location>::iterator			getLocationKey(std::string key);

		int               								getFd() const;
		void                                    		setFd(int);
		
		void                                    		initErrorPages(void);
		static void                             		checkSemicolon(std::string &token);
		bool                                    		checkLocations() const;
	
		void 											setUpIndividualServer(void);
		
		class ErrorException : public std::exception
		{
			private:
				std::string _message;

			public:
				ErrorException(std::string message) throw() {
					_message = ERROR_SERV_PARSER + message;
				}
				virtual const char* what() const throw() {
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif
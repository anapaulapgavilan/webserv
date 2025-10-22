#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "WebServ.hpp"

class Request
{
	private:
		std::string									_method;
		std::string									_version;
		std::map<std::string, std::string>			_headers;
		int											_ret; // return number?
		std::string									_body;
		int											_port;
		std::string									_path;
		std::string									_query;
		std::list<std::pair<std::string, float> >	_lang;
		const std::string&							_raw;
		bool										_autoindex; // to generate index (file list) if no index file found. False by default.
		const Location*								_matched_location; // best matching location for this request

		/*** PARSING ***/
		int			read_first_line(const std::string& line);
		int			readPath(const std::string& line, size_t i);
		int			readVersion(const std::string& line, size_t i);
		int			checkMethod();
		int			checkPort();
		std::string	nextLine(const std::string &str, size_t& i);
		void		setLang();


		/*** AVAILABLE HTTP METHODS ***/
		static	std::vector<std::string>	methods;

		/*** UNAVAILABLE CTORS ***/
		Request();
		Request(const Request&);

	public:
		Request(const std::string& str);
		~Request();
		Request&	operator=(const Request&);

		/*** GETTERS ***/
		const std::map<std::string, std::string>&			getHeaders() const;
		const std::string&									getMethod() const;
		const std::string&									getVersion() const;
		int													getRet() const;
		const std::string&									getBody() const;
		int													getPort() const;
		const std::string&									getPath() const;
		const std::string&									getQuery() const;
		const std::string&									getRaw() const;
		const std::list<std::pair<std::string, float> >&	getLang() const;
		const bool&											getAutoindex() const;
		const Location*										getMatchedLocation() const;

		/*** SETTERS **/
		void	setBody(const std::string& line);
		void	setRet(int);
		void	setMethod(const std::string &method);
		void	setPath(const std::string &new_path);
		void	setAutoindex(bool ai);
		void	setMatchedLocation(const Location *loc);

		/*** UTILS ****/
		int		parse(const std::string& str);
		void	resetHeaders();
		void	stripAll();
		void	findQuery();

		static std::vector<std::string>		init_methods();

};

std::ostream&	operator<<(std::ostream& os, const Request& re);

#endif
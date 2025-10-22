#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "../include/WebServ.hpp"

#define ERR_ROOT_ALIAS "Error: Alias and Root cannot coexist in the same location"
#define ERR_ROOT_LOCATION "Error: Root must be a valid directory"
#define ERR_SUPPORT_METHOD "Error: Allow Method not Supported "

class Location
{
	private:
		std::string							_path;
		std::string							_root;
		bool								_autoindex; // default; off - no listing, use index or 403
		std::string							_index;
		std::vector<short>					_methods;
		std::string							_return;
		std::string							_alias;
		unsigned long						_client_max_body_size;
		std::map<std::string, std::string>	_cgi_ext_map; // map extension -> cgi path

	public:
		Location();
		Location(const Location &other);
		Location &operator=(const Location &rhs);
		~Location();


		void                                        setPathLocation(std::string token);
		void                                        setRootLocation(std::string token);
		void                                        setMethods(std::vector<std::string> methods);
		void                                        setAutoindex(std::string token);
		void                                        setIndexLocation(std::string token);
		void                                        setReturn(std::string token);
		void                                        setAlias(std::string token);
		void                                        setMaxBodySize(std::string token);
		void                                        setMaxBodySize(unsigned long token);

		const std::string                           &getPathLocation() const;
		const std::string                           &getRootLocation() const;
		const std::vector<short>                    &getMethods() const;
		const bool                                  &getAutoindex() const;
		const std::string                           &getIndexLocation() const;
		const std::string                           &getReturn() const;
		const std::string                           &getAlias() const;
		const std::map<std::string, std::string>    &getCgiExtMap() const;
		const unsigned long                         &getMaxBodySize() const;
		std::string                                 getPrintMethods() const;

		void addCgiHandler(const std::string &ext, const std::string &path);
		const std::string &getCgiHandler(const std::string &ext) const;
};

#endif
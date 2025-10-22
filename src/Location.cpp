#include "../include/WebServ.hpp"


Location::Location()
{
	this->_path = "";
	this->_root = "";
	this->_autoindex = false;
	this->_index = "";
	this->_return = "";
	this->_alias = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_methods.reserve(5);
	this->_methods.push_back(1); // GET enabled by default
	this->_methods.push_back(0);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
}

Location::Location(const Location &other)
{
	this->_path = other._path;
	this->_root = other._root;
	this->_autoindex = other._autoindex;
	this->_index = other._index;
	this->_return = other._return;
	this->_alias = other._alias;
    this->_methods = other._methods;
	this->_cgi_ext_map = other._cgi_ext_map;
	this->_client_max_body_size = other._client_max_body_size;
}

Location &Location::operator=(const Location &rhs)
{
	if (this != &rhs)
	{
		this->_path = rhs._path;
		this->_root = rhs._root;
		this->_autoindex = rhs._autoindex;
		this->_index = rhs._index;
		this->_return = rhs._return;
		this->_alias = rhs._alias;
		this->_methods = rhs._methods;
		this->_cgi_ext_map = rhs._cgi_ext_map;
		this->_client_max_body_size = rhs._client_max_body_size;
    }
	return (*this);
}

Location::~Location() { }

void Location::setPathLocation(std::string token)
{
	// si no es /, pero acaba en /, quitar el /
	if (token != "/" && token[token.length() - 1] == '/')
		token = token.substr(0, token.length() - 1);
	
	this->_path = token;
}

void Location::setRootLocation(std::string token)
{
	if (ConfigFile::getTypePath(token) != F_DIRECTORY)
		throw ServerUnit::ErrorException(ERR_ROOT_LOCATION);
	if (Location::getAlias() != "")
		throw ServerUnit::ErrorException(ERR_ROOT_ALIAS);
	this->_root = token;
}

void Location::setMethods(std::vector<std::string> methods)
{
	this->_methods[0] = 0;
	this->_methods[1] = 0;
	this->_methods[2] = 0;
	this->_methods[3] = 0;
	this->_methods[4] = 0;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			this->_methods[M_GET] = 1;
		else if (methods[i] == "POST")
			this->_methods[M_POST] = 1;
		else if (methods[i] == "DELETE")
			this->_methods[M_DELETE] = 1;
		else if (methods[i] == "PUT")
			this->_methods[M_PUT] = 1;
		else if (methods[i] == "HEAD")
			this->_methods[M_HEAD] = 1;
		else
			throw ServerUnit::ErrorException(ERR_SUPPORT_METHOD + methods[i]);
	}
}

void Location::setAutoindex(std::string token)
{
	if (token == "on" || token == "off")
		this->_autoindex = (token == "on");
	else
		throw ServerUnit::ErrorException(AUTOINDEX_ERR ": must be 'on' or 'off'");
}

void Location::setIndexLocation(std::string token)
{
	this->_index = token;
}

void Location::setReturn(std::string token)
{
	this->_return = token;
}

void Location::setAlias(std::string token)
{
	if (Location::getRootLocation() != "")
		throw ServerUnit::ErrorException(ERR_ROOT_ALIAS);
	this->_alias = token;
}

void Location::setMaxBodySize(std::string token)
{
	unsigned long body_size = 0;

	for (size_t i = 0; i < token.length(); i++)
	{
		if (token[i] < '0' || token[i] > '9')
			throw ServerUnit::ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	}
	if (!ft_stoi(token))
		throw ServerUnit::ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
	body_size = ft_stoi(token);
	this->_client_max_body_size = body_size;
}

void Location::setMaxBodySize(unsigned long token)
{
	this->_client_max_body_size = token;
}

const std::string &Location::getPathLocation() const
{
	return (this->_path);
}

const std::string &Location::getRootLocation() const
{
	return (this->_root);
}

const std::string &Location::getIndexLocation() const
{
	return (this->_index);
}

const std::vector<short> &Location::getMethods() const
{
	return (this->_methods);
}

const bool &Location::getAutoindex() const
{
	return (this->_autoindex);
}

const std::string &Location::getReturn() const
{
	return (this->_return);
}

const std::string &Location::getAlias() const
{
	return (this->_alias);
}

const std::map<std::string, std::string> &Location::getCgiExtMap() const
{
	return (this->_cgi_ext_map);
}

const unsigned long &Location::getMaxBodySize() const
{
	return (this->_client_max_body_size);
}

std::string Location::getPrintMethods() const
{
	std::string res;
    bool first = true;

    // iteramos sobre todos los métodos posibles
    for (int i = M_GET; i <= M_HEAD; ++i) 
    {
        if (_methods[i]) 
        {
            if (!first)
                res += ", ";
            res += method_toString(i);
            first = false;
        }
    }

    return res;
}

void Location::addCgiHandler(const std::string &ext, const std::string &path) {
	_cgi_ext_map[ext] = path;
}

const std::string &Location::getCgiHandler(const std::string &ext) const {
    static const std::string empty = "";
    std::map<std::string, std::string>::const_iterator it = _cgi_ext_map.find(ext);
    if (it == _cgi_ext_map.end())
        return empty;
    return it->second;
}

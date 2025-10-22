#include "../include/WebServ.hpp"

ReadConfig::ReadConfig()
{
	this->_nb_server = 0;
}

ReadConfig::~ReadConfig() { }

int ReadConfig::createServerGroup(const std::string &config_file)
{
	std::string		content;
	ConfigFile		file(config_file);

	if (file.getTypePath(file.getPath()) != F_REGULAR_FILE)
		throw ErrorException(INVALID_FILE_ERR);
	if (!file.checkFile(file.getPath(), R_OK))
		throw ErrorException(ACCESS_FILE_ERR);
	content = file.readFile(config_file);
	if (content.empty())
		throw ErrorException(EMPTY_FILE_ERR);
	removeComments(content);
	removeWhiteSpace(content);
	splitServers(content);
	if (this->_server_config.size() != this->_nb_server)
		throw ErrorException(SIZE_FILE_ERR);
	for (size_t i = 0; i < this->_nb_server; i++)
	{
		ServerUnit server;
		createServer(this->_server_config[i], server);
		logDebug("🍉 Server #%i created", i);
		this->_servers.push_back(server);
	}
	if (this->_nb_server > 1)
		checkServers();
	return (0);
}

void ReadConfig::removeComments(std::string &content)
{
	size_t pos;

	pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t pos_end;
		pos_end = content.find('\n', pos);
		content.erase(pos, pos_end - pos);
		pos = content.find('#');
	}
}

void ReadConfig::removeWhiteSpace(std::string &content)
{
	size_t	i = 0;

	while (content[i] && isspace(content[i]))
		i++;
	content = content.substr(i);
	i = content.length() - 1;
	while (i > 0 && isspace(content[i]))
		i--;
	content = content.substr(0, i + 1);
}

void ReadConfig::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	if (content.find("server", 0) == std::string::npos)
		throw ErrorException(FIND_FILE_ERR);
	while (start != end && start < content.length())
	{
		start = findStartServer(start, content);
		end = findEndServer(start, content);
		if (start == end)
			throw ErrorException(SCOPE_FILE_ERR);
		this->_server_config.push_back(content.substr(start, end - start + 1));
		this->_nb_server++;
		start = end + 1;
	}
}

size_t ReadConfig::findStartServer (size_t start, std::string &content)
{
	size_t i;

	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break ;
		if (!isspace(content[i]))
			throw ErrorException(SERVER_SCOPE_ERR);
	}
	if (!content[i])
		return (start);
	if (content.compare(i, 6, "server") != 0)
		throw ErrorException(SERVER_SCOPE_ERR);
	i += 6;
	while (content[i] && isspace(content[i]))
		i++;
	if (content[i] == '{')
		return (i);
	else
		throw ErrorException(SERVER_SCOPE_ERR);

}

size_t ReadConfig::findEndServer (size_t start, std::string &content)
{
	size_t	i;
	size_t	scope;
	
	scope = 0;
	for (i = start + 1; content[i]; i++)
	{
		if (content[i] == '{')
			scope++;
		if (content[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return (start);
}

std::vector<std::string> splitTokens(const std::string& line, const std::string& separators) 
{
    std::vector<std::string> tokens;
    size_t start = line.find_first_not_of(separators), end = 0;
    while ((end = line.find_first_of(separators, start)) != std::string::npos) {
        tokens.push_back(line.substr(start, end - start));
        start = line.find_first_not_of(separators, end);
    }
    if (start != std::string::npos)
        tokens.push_back(line.substr(start));
    return tokens;
}


void ReadConfig::createServer(std::string &config, ServerUnit &server)
{
	std::vector<std::string>	tokens;
	std::vector<std::string>	error_codes;
	int		flag_loc = 1;
	bool	flag_autoindex = false;
	bool	flag_max_size = false;

	tokens = splitTokens(config += ' ', std::string(" \n\t"));
	if (tokens.size() < 3)
		throw ErrorException(SERVER_VALIDATION_ERR);
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i] == "listen" && (i + 1) < tokens.size() && flag_loc)
		{
			if (server.getPort())
				throw ErrorException(PORT_ERR);
			server.setPort(tokens[++i]);
		}
		else if (tokens[i] == "location" && (i + 1) < tokens.size())
		{
			std::string	path;
			i++;
			if (tokens[i] == "{" || tokens[i] == "}")
				throw ErrorException(SERVER_SCOPE_ERR);
			path = tokens[i];
			std::vector<std::string> codes;
			if (tokens[++i] != "{")
				throw ErrorException(SERVER_SCOPE_ERR);
			i++;
			while (i < tokens.size() && tokens[i] != "}")
				codes.push_back(tokens[i++]);
			server.setLocation(path, codes);
			if (i < tokens.size() && tokens[i] != "}")
				throw ErrorException(SERVER_SCOPE_ERR);
			flag_loc = 0;
		}
		else if (tokens[i] == "host" && (i + 1) < tokens.size() && flag_loc)
		{
			if (server.getHost())
				throw ErrorException(HOST_ERR);
			server.setHost(tokens[++i]);
		}
		else if (tokens[i] == "root" && (i + 1) < tokens.size() && flag_loc)
		{
			if (!server.getRoot().empty())
				throw ErrorException(ROOT_ERR);
			server.setRoot(tokens[++i]);
		}
		else if (tokens[i] == "error_page" && (i + 1) < tokens.size() && flag_loc)
		{
			while (++i < tokens.size())
			{
				error_codes.push_back(tokens[i]);
				if (tokens[i].find(';') != std::string::npos)
					break ;
				if (i + 1 >= tokens.size())
					throw ErrorException(SERVER_SCOPE_ERR);
			}
		}
		else if (tokens[i] == "client_max_body_size" && (i + 1) < tokens.size() && flag_loc)
		{
			if (flag_max_size)
				throw ErrorException(CLIENT_ERR);
			server.setClientMaxBodySize(tokens[++i]);
			flag_max_size = true;
		}
		else if (tokens[i] == "server_name" && (i + 1) < tokens.size() && flag_loc)
		{
			if (!server.getServerName().empty())
				throw ErrorException(SERVER_NAME_ERR);
			server.setServerName(tokens[++i]);
		}
		else if (tokens[i] == "index" && (i + 1) < tokens.size() && flag_loc)
		{
			if (!server.getIndex().empty())
				throw ErrorException(INDEX_ERR);
			server.setIndex(tokens[++i]);
		}
		else if (tokens[i] == "autoindex" && (i + 1) < tokens.size() && flag_loc)
		{
			if (flag_autoindex)
				throw ErrorException(AUTOINDEX_ERR ": is duplicated");
			server.setAutoindex(tokens[++i]);
			flag_autoindex = true;
		}
		else if (tokens[i] != "}" && tokens[i] != "{")
		{
			if (!flag_loc)
				throw ErrorException(TOKEN_ERR ": " + tokens[i] + "(unknown token in location)");
			else
				throw ErrorException(DIRECTIVE_ERR);
		}
	}
	// set default values if not set
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (server.getHost() == 0)
		server.setHost("localhost;");
	// if (server.getIndex().empty())
	// 	server.setIndex("index.html;");
	
	// check server index
	if (!ConfigFile::isFileExistAndReadable(server.getRoot()+"/", server.getIndex()))
		throw ErrorException(INDEX_CONFIG_ERR);	
	
	if (server.checkLocations())
		throw ErrorException(LOCATION_DUP_ERR);
	if (!server.getPort())
		throw ErrorException(PORT_ERR);
	server.setErrorPages(error_codes);
	if (!server.isValidErrorPages())
		throw ErrorException(ERROR_PAGE_ERR);
}

void ReadConfig::checkServers()
{
	std::vector<ServerUnit>::iterator it1;
	std::vector<ServerUnit>::iterator it2;

	for (it1 = this->_servers.begin(); it1 != this->_servers.end() - 1; it1++)
	{
		for (it2 = it1 + 1; it2 != this->_servers.end(); it2++)
		{
			if (it1->getPort() == it2->getPort() && it1->getHost() == it2->getHost() && it1->getServerName() == it2->getServerName())
				throw ErrorException(SERVER_ERR);
		}
	}
}

std::vector<ServerUnit>	ReadConfig::getServers()
{
	return (this->_servers);
}
#include "../include/WebServ.hpp"

ServerUnit::ServerUnit() // Check
{
    this->_port = 0;
    this->_host = 0;
    this->_server_name = "";
    this->_root = "";
    this->_client_max_body_size = MAX_CONTENT_LENGTH;
    this->_index = "";
    this->_autoindex = false;
    this->initErrorPages();
    this->_listen_fd = 0;
}

ServerUnit::~ServerUnit() { } // Check

ServerUnit::ServerUnit(const ServerUnit &other) // Check
{
    if (this != &other)
    {
        this->_server_name = other._server_name;
        this->_root = other._root;
        this->_host = other._host;
        this->_port = other._port;
        this->_client_max_body_size = other._client_max_body_size;
        this->_index = other._index;
        this->_error_list = other._error_list;
        this->_locations = other._locations;
        this->_listen_fd = other._listen_fd;
        this->_autoindex = other._autoindex;
        this->_server_address = other._server_address;
    }
    return ;
}

ServerUnit &ServerUnit::operator=(const ServerUnit & rhs) // Check
{
    if (this != &rhs)
    {
        this->_server_name = rhs._server_name;
        this->_root = rhs._root;
        this->_port = rhs._port;
        this->_host = rhs._host;
        this->_client_max_body_size = rhs._client_max_body_size;
        this->_index = rhs._index;
        this->_error_list = rhs._error_list;
        this->_locations = rhs._locations;
        this->_listen_fd = rhs._listen_fd;
        this->_autoindex = rhs._autoindex;
        this->_server_address = rhs._server_address;
    }
    return (*this);
}

void ServerUnit::initErrorPages(void) // Check
{
    _error_list[301] = "";
    _error_list[302] = "";
    _error_list[400] = "";
    _error_list[401] = "";
    _error_list[402] = "";
    _error_list[403] = "";
    _error_list[404] = "";
    _error_list[405] = "";
    _error_list[406] = "";
    _error_list[500] = "";
    _error_list[501] = "";
    _error_list[502] = "";
    _error_list[503] = "";
    _error_list[505] = "";
    _error_list[505] = "";
}

void ServerUnit::setServerName(std::string server_name) // Check
{
    checkSemicolon(server_name);
    this->_server_name = server_name;
}

void ServerUnit::setHost(std::string token) // Check
{
    checkSemicolon(token);
    if (token == "localhost")
        token = "127.0.0.1";
    if (!isValidHost(token))
        throw ErrorException(SYNTAX_ERR_HOST);
    this->_host = inet_addr(token.data());
}

void ServerUnit::setRoot(std::string root) // Check 
{
    checkSemicolon(root);
    if (ConfigFile::getTypePath(root) == F_DIRECTORY)
    {
        this->_root = root;
        return ;
    }
    // if it is not a directory:
    char dir[1024];
    getcwd(dir, 1024);
    std::string full_root = dir + root;
    if (ConfigFile::getTypePath(full_root) != F_DIRECTORY)
        throw ErrorException(SYNTAX_ERR_ROOT);
    this->_root = full_root;
    logError("root is not a directory - averigua que pasa en este caso");
    exit(2);
}

void ServerUnit::setPort(std::string token) // Check 
{
    unsigned int port;
    
    port = 0;
    checkSemicolon(token);
    for (size_t i = 0; i < token.length(); i++)
    {
        if (!std::isdigit(token[i]))
            throw ErrorException(SYNTAX_ERR_PORT);
    }
    port = ft_stoi((token));
    if (port < 1 || port > 65636)
        throw ErrorException(SYNTAX_ERR_PORT);
    this->_port = (uint16_t) port;
}

void ServerUnit::setClientMaxBodySize(std::string token) // Check
{
    unsigned long body_size;
    
    body_size = 0;
    checkSemicolon(token);
    for (size_t i = 0; i < token.length(); i++)
    {
        if (token[i] < '0' || token[i] > '9')
            throw ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
    }
    if (!ft_stoi(token))
        throw ErrorException(SYNTAX_ERR_CLIENT_MAX_SIZE);
    body_size = ft_stoi(token);
    this->_client_max_body_size = body_size;
}

void ServerUnit::setIndex(std::string index) //Check 
{
    checkSemicolon(index);
    // check it does not start with /
    if (index[0] == '/')
        throw ErrorException(SYNTAX_ERR_INDEX ": cannot start with /");
    this->_index = index;
}

void ServerUnit::setAutoindex(std::string autoindex) //Check
{
    checkSemicolon(autoindex);
    if (autoindex != "on" && autoindex != "off")
        throw ErrorException(SYNTAX_ERR_AUTOINDEX);
    if (autoindex == "on")
        this->_autoindex = true;
}

void	ServerUnit::setFd(int fd) //Check
{
    this->_listen_fd = fd;
}

void ServerUnit::setErrorPages(std::vector<std::string> &token) // Check
{
    if (token.empty())
        return;
    if (token.size() % 2 != 0)
        throw ErrorException (PAGE_ERR_INIT);
    for (size_t i = 0; i < token.size() - 1; i++)
    {
        for (size_t j = 0; j < token[i].size(); j++) {
            if (!std::isdigit(token[i][j]))
                throw ErrorException(PAGE_ERR_CODE);
        }
        if (token[i].size() != 3)
            throw ErrorException(PAGE_ERR_CODE);
        short code_error = ft_stoi(token[i]);
        if (statusCodeString(code_error) == UNDEFINED|| code_error < 400)
            throw ErrorException (INCORRECT_ERR_CODE + token[i]);
        i++;
        std::string path = token[i];
        checkSemicolon(path);
        if (ConfigFile::getTypePath(path) != F_DIRECTORY)
        {
            if (ConfigFile::getTypePath(this->_root + path) != F_REGULAR_FILE)
                throw ErrorException (INCORRECT_ERR_PATH + this->_root + path);
            if (!ConfigFile::checkFile(this->_root + path, F_OK) || !ConfigFile::checkFile(this->_root + path, R_OK))
                throw ErrorException (INCORRECT_ERR_PATH + this->_root + path + UNACCESSIBLE);
        }
        std::map<short, std::string>::iterator it = this->_error_list.find(code_error);
        if (it != _error_list.end())
            this->_error_list[code_error] = path;
        else
            this->_error_list.insert(std::make_pair(code_error, path));
    }
}

void ServerUnit::setLocation(std::string path, std::vector<std::string> tokens) // Check
{
    Location new_location;

    std::vector<std::string> methods;
    bool flag_methods = false;
    bool flag_autoindex = false;
    bool flag_max_size = false;
    int valid;

    new_location.setPathLocation(path);
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i] == ROOT && (i + 1) < tokens.size())
        {
            if (!new_location.getRootLocation().empty())
                throw ErrorException(ROOT_DUP_ERR);
            checkSemicolon(tokens[++i]);
            if (ConfigFile::getTypePath(tokens[i]) == F_DIRECTORY)
                new_location.setRootLocation(tokens[i]);
            else
            {
                logDebug("setting Root location to: %s", tokens[i].c_str());
                exit(1) ; // TODO: quiero entender esto          
                new_location.setRootLocation(this->_root + tokens[i]);
            }
        }
        else if ((tokens[i] == ALLOW_METHODS || tokens[i] == METHODS) && (i + 1) < tokens.size())
        {
            if (flag_methods)
                throw ErrorException(METHODS_DUP_ERR);
            std::vector<std::string> methods;
            while (++i < tokens.size())
            {
                if (tokens[i].find(";") != std::string::npos)
                {
                    checkSemicolon(tokens[i]);
                    methods.push_back(tokens[i]);
                    break ;
                }
                else
                {
                    methods.push_back(tokens[i]);
                    if (i + 1 >= tokens.size())
                        throw ErrorException(TOKEN_ERR ": " + tokens[i] + "(missing semicolon)");    
                }
            }
            new_location.setMethods(methods);
            flag_methods = true;
        }
        else if (tokens[i] == AUTOINDEX && (i + 1) < tokens.size())
        {
            if (path == CGI_BIN_PATH)
                throw ErrorException(AUTOINDEX_ERR ": cannot use cgi-bin");
            if (flag_autoindex)
                throw ErrorException(AUTOINDEX_DUP_ERR);
            checkSemicolon(tokens[++i]);
            new_location.setAutoindex(tokens[i]);
            flag_autoindex = true;
        }
        else if (tokens[i] == INDEX && (i + 1) < tokens.size())
        {
            if (!new_location.getIndexLocation().empty())
                throw ErrorException(INDEX_DUP_ERR);
            checkSemicolon(tokens[++i]);
            // check it does not start with /
            if (tokens[i][0] == '/')
                throw ErrorException(SYNTAX_ERR_INDEX ": cannot start with /");
            new_location.setIndexLocation(tokens[i]);
        }
        else if (tokens[i] == RETURN && (i + 1) < tokens.size())
        {
            if (path == CGI_BIN_PATH)
                throw ErrorException(RETURN_ERR_CGI);
            if (!new_location.getReturn().empty())
                throw ErrorException(RETURN_DUP_ERR);
            checkSemicolon(tokens[++i]);
            new_location.setReturn(tokens[i]);
        }
        else if (tokens[i] == ALIAS && (i + 1) < tokens.size())
        {
            if (path == CGI_BIN_PATH)
                throw ErrorException(ALIAS_ERR_CGI);
            if (!new_location.getAlias().empty())
                throw ErrorException(ALIAS_DUP_ERR);
            checkSemicolon(tokens[++i]);
            new_location.setAlias(tokens[i]);
        }
        else if (tokens[i] == CGI)
        {
            // expected: cgi <extension> <cgi_path> ;
            if (i + 2 >= tokens.size())
                throw ErrorException(TOKEN_ERR ": incomplete cgi directive");

            std::string extension = tokens[i + 1];
            std::string cgi_path  = tokens[i + 2];

            // Validar formato del path con ';' al final
            checkSemicolon(cgi_path); // verfica y elimina ';' al final

            // check syntax
            if (extension.size() < 2 || extension[0] != '.' )
            throw ErrorException(SYNTAX_ERR_CGI_EXT);
            if (cgi_path.size() < 3 || cgi_path[0] != '.' || cgi_path[1] != '/')
                throw ErrorException(SYNTAX_ERR_CGI_PATH);
            
            cgi_path = cgi_path.substr(2); // quitar './' al inicio

            // check dups
            if (new_location.getCgiHandler(extension) != "")
                throw ErrorException(CGI_EXT_DUP_ERR + extension);
            
            // add the asociation to the map in location
            new_location.addCgiHandler(extension, cgi_path);
            i += 2; // skip the processed tokens
        }
        else if (tokens[i] == CMBS && (i + 1) < tokens.size())
        {
            if (flag_max_size)
                throw ErrorException(CMBS_DUP_ERR);
            checkSemicolon(tokens[++i]);
            new_location.setMaxBodySize(tokens[i]);
            flag_max_size = true;
        }
        else if (i < tokens.size())
            throw ErrorException(TOKEN_ERR ": " + tokens[i] + "(unknown token)");
    }
    // if (new_location.getPathLocation() != CGI_BIN_PATH && new_location.getIndexLocation().empty())
    //     new_location.setIndexLocation(this->_index);
    if (!flag_max_size)
        new_location.setMaxBodySize(this->_client_max_body_size);
    valid = isValidLocation(new_location);
    if (valid == ER_VAL_CGI)
        throw ErrorException(CGI_ERR_VALIDATION);
    else if (valid == ER_VAL_CGI_PATH_NONEXE)
        throw ErrorException(CGI_ERR_VALIDATION ": cgi path does not exist or is not executable");
    else if (valid == ER_VAL_LOCATION)
        throw ErrorException(LOCATION_ERR_VALIDATION);
    else if (valid == ER_VAL_REDIRECT)
        throw ErrorException(REDIRECTION_ERR_VALIDATION);
    else if (valid == ER_VAL_ALIAS)
        throw ErrorException(ALIAS_ERR_VALIDATION);
    else if (valid == ER_VAL_INDEX) // check
        throw ErrorException(INDEX_ERR_VALIDATION);
    this->_locations.push_back(new_location);
}

int isValidCgiLocation(Location &location)
{
    // for each extension and cgi path, check if they are valid
    const std::map<std::string, std::string> &ext_map = location.getCgiExtMap();
    std::map<std::string, std::string>::const_iterator it;
    for (it = ext_map.begin(); it != ext_map.end(); ++it) {
        std::string ext = it->first;
        std::string path = it->second;
        // Check if cgi path file exists and is executable
        if (!ConfigFile::isFileExistAndExecutable("./", path))				
            return (ER_VAL_CGI_PATH_NONEXE);
    } 
    return (0);
}

int ServerUnit::isValidLocation(Location &location) const // Check
{
    if (location.getPathLocation() == CGI_BIN_PATH)
    {
        // Special validation for cgi-bin location
        /**
         * case:
         * location /cgi-bin/ {...
         */
        logError("Validating cgi-bin location: not supported");
        exit(1);

    }
    else
    {
        // logDebug("Validating location: root:%s", location.getRootLocation().c_str());
        // logDebug("Validating location: path:%s", location.getPathLocation().c_str());

        // Location must start with /
        if (location.getPathLocation()[0] != '/')
            return (ER_VAL_LOCATION);

        // // if root is empty, set it to server root
        // if (location.getRootLocation().empty())
        // {
        //     logDebug("1 setting Root location to: %s", this->_root.c_str());
        //     location.setRootLocation(this->_root);
        // }
        // // root must be a directory
        // if (ConfigFile::getTypePath(location.getRootLocation()) != F_DIRECTORY)
        //     return (ER_VAL_LOCATION);

        // if (!location.getIndexLocation().empty())
        // {
        //     logDebug("Validating index in location: index:%s", location.getIndexLocation().c_str());
        //     if (!ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPathLocation() + "/", location.getIndexLocation()))
        //         return (ER_VAL_INDEX); //check
        // }

        // if (!location.getReturn().empty())
        // {
        //     logDebug("Validating location: return:%s", location.getReturn().c_str());
        //     if (!ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPathLocation(), location.getReturn()))
        //         return (ER_VAL_REDIRECT);
        // }


        // if (!location.getAlias().empty())
        // {
        //     logDebug("Validating location: alias:%s", location.getAlias().c_str());
        //     if (!ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPathLocation() + "/", location.getAlias()))
        //          return (ER_VAL_ALIAS);
        // }
        int cgi_status = isValidCgiLocation(location);

        if (cgi_status != 0)
            return (cgi_status);
    }
    return (0);
}

bool ServerUnit::isValidHost(std::string host) const // Check
{
    struct sockaddr_in sockaddr;
    if (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)))
        return true;
    else
        return false;
}

bool ServerUnit::isValidErrorPages() // Check
{
    std::map<short, std::string>::const_iterator it;
    for (it = this->_error_list.begin(); it != this->_error_list.end(); it++)
    {
        if (it->first < 100 || it->first > 599)
            return (false);
        if (!ConfigFile::checkFile(getRoot() + it->second, F_OK) || !ConfigFile::checkFile(getRoot() + it->second, R_OK))
            return (false);
    }
    return (true);
}

const std::string &ServerUnit::getServerName() //Check
{
    return (this->_server_name);
}

const std::string &ServerUnit::getRoot() //Check
{
    return (this->_root);
}

const bool &ServerUnit::getAutoindex() //Check
{
    return (this->_autoindex);
}

const in_addr_t &ServerUnit::getHost() const { return this->_host; }

const uint16_t &ServerUnit::getPort() const  { return this->_port; }

const size_t &ServerUnit::getClientMaxBodySize() //Check
{
    return (this->_client_max_body_size);
}

const std::vector<Location> &ServerUnit::getLocations() //Check
{
    return (this->_locations);
}

const std::map<short, std::string> &ServerUnit::getErrorPages() //Check
{
    return (this->_error_list);
}

const std::string &ServerUnit::getIndex() //Check
{
    return (this->_index);
}

int ServerUnit::getFd() const { return this->_listen_fd; }

/**
 * Returns the path of the error page for a given HTTP status code.
 * If no custom error page is set for the given code, an empty string is returned.
 */
const std::string &ServerUnit::getPathErrorPage(short key) // Check
{
    std::map<short, std::string>::iterator it = this->_error_list.find(key);
    if (it == this->_error_list.end())
    {
        static const std::string empty = ""; // no se destruirá cuando acabe la función
        return empty;
    }
    return (it->second);
}

const std::vector<Location>::iterator ServerUnit::getLocationKey(std::string key) // Check 
{
    std::vector<Location>::iterator it;
    for (it = this->_locations.begin(); it != this->_locations.end(); it++)
    {
        if (it->getPathLocation() == key)
            return (it);
    }
    throw ErrorException(LOCATION_ERR);
}

/**
 * checkSemicolon: Checks if the given token ends with a semicolon
 * and delete the semicolon.
 */
void ServerUnit::checkSemicolon(std::string &token) // Check 
{
    size_t pos = token.rfind(';');
    if (pos != token.size() - 1)
        throw ErrorException(TOKEN_ERR ": " + token + " (missing semicolon)");
    token.erase(pos);
}

bool ServerUnit::checkLocations() const // Check
{
    if (this->_locations.size() < 2)
        return (false);
    std::vector<Location>::const_iterator it1;
    std::vector<Location>::const_iterator it2;
    for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
        for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
            if (it1->getPathLocation() == it2->getPathLocation())
                return (true);
        }
    }
    return (false);
}

void ServerUnit::setUpIndividualServer()
{
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd == -1)
        throw ErrorException(std::string(SOCKET_ERR) + strerror(errno));

    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw ErrorException(std::string(SET_SOCKET_ERR) + strerror(errno));
        
    set_nonblocking(_listen_fd);

    memset(&_server_address, 0, sizeof(_server_address));
    _server_address.sin_family = AF_INET;
    _server_address.sin_addr.s_addr = _host;
    // _server_address.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any IP
    _server_address.sin_port = htons(_port); 

    if (bind(_listen_fd, reinterpret_cast<sockaddr*>(&_server_address),
             sizeof(_server_address)) == -1)
        throw ErrorException(std::string(BIND_ERR) + strerror(errno));
}


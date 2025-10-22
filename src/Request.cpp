#include "../include/WebServ.hpp"


std::vector<std::string>		Request::init_methods()
{
	std::vector<std::string>	methods;

	methods.push_back("GET");
	methods.push_back("HEAD");
	methods.push_back("POST");
	methods.push_back("PUT");
	methods.push_back("DELETE");
	methods.push_back("OPTIONS");
	methods.push_back("TRACE");

	return methods;
}

std::vector<std::string>	Request::methods = Request::init_methods();

Request::Request(const std::string& str) :
	_method (""), _version(""), _ret(200), _body(""), _port(80), _path(""), _query(""), _raw(str), _autoindex(false), _matched_location(NULL)
{
	this->resetHeaders();
	this->parse(str);
	if (this->_ret != 200)
		logError("Parse error : %d", this->_ret);
}

Request::~Request()
{
}

Request&	Request::operator=(const Request& obj)
{
	this->_headers = obj.getHeaders();
	this->_method = obj.getMethod();
	this->_version = obj.getVersion();
	this->_ret = obj.getRet();
	this->_body = obj.getBody();
	this->_port = obj.getPort();
	this->_path = obj.getPath();
	this->_query = obj.getQuery();

	return *this;
}

/*** GETTERS ***/

const std::map<std::string, std::string>&	Request::getHeaders() const
{
	return this->_headers;
}

const std::string&	Request::getMethod() const
{
	return this->_method;
}

const std::string&	Request::getVersion() const
{
	return this->_version;
}

int					Request::getRet() const
{
	return this->_ret;
}

const std::string&	Request::getBody() const
{
	return this->_body;
}

int					Request::getPort() const
{
	return this->_port;
}

const std::string&			Request::getPath() const
{
	return this->_path;
}

const std::string&			Request::getQuery() const
{
	return this->_query;
}

const std::string&			Request::getRaw() const
{
	return this->_raw;
}

const std::list<std::pair<std::string, float> >&	Request::getLang() const
{
	return this->_lang;
}

const bool&	Request::getAutoindex() const
{
	return this->_autoindex;
}

const Location* Request::getMatchedLocation() const
{
	return this->_matched_location;
}
/*** SETTERS ***/

void	Request::setBody(const std::string& str)
{
	char	strip[] = {'\n', '\r'};

	this->_body.assign(str);
	for (int i = 0; i < 4; i++)
		if (this->_body.size() > 0 && this->_body[this->_body.size() - 1] == strip[i % 2])
			pop(this->_body);
		else
			break ;
}

void	Request::setRet(int ret)
{
	this->_ret = ret;
}

void	Request::setMethod(const std::string &method)
{
	this->_method = method;
}

void	Request::setPath(const std::string &new_path)
{
	if (new_path.empty()) {
		logError("setPath: invalid path: <%s>", new_path.c_str());
		this->_ret = 400;
		throw HttpException(HttpStatusCode::BadRequest);
	}
	this->_path = new_path;
}

void	Request::setAutoindex(bool ai)
{
	this->_autoindex = ai;
}

void	Request::setMatchedLocation(const Location *loc)
{
	this->_matched_location = loc;
}
void				Request::resetHeaders()
{
	this->_headers.clear();

	this->_headers["Accept-Charsets"] = ""; //ya no se usa, esta obsoleto
	this->_headers["Accept-Language"] = ""; // es, en, ... Client
	this->_headers["Allow"] = ""; // "GET, POST, PUT, DELETE, OPTIONS, HEAD, TRACE"
	this->_headers["Authorization"] = "";
	this->_headers["Content-Language"] = ""; //es, en, ... Server
	this->_headers["Content-Length"] = ""; //longitud del cuerpo de la peticion
	this->_headers["Content-Location"] = ""; // la URL del recurso
	this->_headers["Content-Type"] = ""; //MIME type, text/html, application/json, image/png, etc.
	this->_headers["Date"] = ""; // Ayuda con cachés, sincronización y diagnóstico
	this->_headers["Host"] = "";
	this->_headers["Last-Modified"] = ""; // Fecha de la última modificación del recurso
	this->_headers["Location"] = "";
	this->_headers["Referer"] = ""; // URL de la página que envió la solicitud actual
	this->_headers["Retry-After"] = ""; // Indica cuánto tiempo (en segundos) debe esperar el cliente antes de volver a enviar la solicitud
	this->_headers["Server"] = ""; // Información sobre el servidor que maneja la solicitud
	this->_headers["Transfer-Encoding"] = ""; // Indica cómo se codifica el cuerpo de la solicitud para la transmisión
	this->_headers["User-Agent"] = ""; // Información sobre el cliente que realiza la solicitud
	this->_headers["Www-Authenticate"] = ""; // Indica el esquema de autenticación que el servidor requiere para acceder al recurso
	this->_headers["Connection"] = "Keep-Alive"; // por defecto los request son Keep-Alive
}

int					Request::parse(const std::string& str)
{
	std::string		key;
	std::string		value;
	std::string		line;
	size_t			i = 0;

	this->read_first_line(nextLine(str, i));
	while ((line = nextLine(str, i)) != "\r" && line != "" && this->_ret != 400)
	{
		key = readKey(line);
        
		value = readValue(line);
		if (this->_headers.count(key))
				this->_headers[key] = value;
	}
	this->setLang();
	this->setBody(str.substr(i, std::string::npos));
	this->checkPort();
	this->findQuery();
	return this->_ret;
}

int					Request::read_first_line(const std::string& str)
{
	size_t	i;
	std::string	line;

	i = str.find_first_of('\n');
	line = str.substr(0, i);
	i = line.find_first_of(' ');

	if (i == std::string::npos)
	{
		this->_ret = 400;
		logError("RFL no space after method. Line: <%s>", line.c_str());
		return 400;
	}
	this->_method.assign(line, 0, i);
	return this->readPath(line, i);
}

void				Request::setLang()
{
	std::vector<std::string>	token;
	std::string					header;
	size_t						i;

	if ((header = this->_headers["Accept-Language"]) != "")
	{
		token = split(header, ',');
		for (std::vector<std::string>::iterator it = token.begin(); it != token.end(); it++)
		{
			float			weight = 0.0;
			std::string		lang;

			lang = (*it).substr(0, (*it).find_first_of('-'));
			strip(lang, ' ');
			if ( (i = lang.find_last_of(';') ) != std::string::npos)
			{
				weight = atof( (*it).substr(i + 4).c_str() );
			}
			lang.resize(i > 2 ? 2 : i);
			this->_lang.push_back(std::pair<std::string, float>(lang, weight));
		}
		this->_lang.sort(compare_langs);
	}
}

void				Request::stripAll()
{
	strip(this->_method, '\n');
	strip(this->_method, '\r');
	strip(this->_method, ' ');
	strip(this->_version, '\n');
	strip(this->_version, '\r');
	strip(this->_version, ' ');
	strip(this->_path, '\n');
	strip(this->_path, '\r');
	strip(this->_path, ' ');
}

std::string			Request::nextLine(const std::string &str, size_t& i)
{
	std::string		ret;
	size_t			j;

	if (i == std::string::npos)
		return "";
	j = str.find_first_of('\n', i);
	ret = str.substr(i, j - i);
	if (ret[ret.size() - 1] == '\r')
		pop(ret);
	i = (j == std::string::npos ? j : j + 1);
	return ret;
}

int					Request::readPath(const std::string& line, size_t size)
{
	size_t	n_spaces;

	// skip spaces 
	if ((n_spaces = line.find_first_not_of(' ', size)) == std::string::npos)
	{
		this->_ret = 400;
		std::cerr << RED << "No PATH / HTTP version" << RESET << std::endl;
		return 400;
	}
	// read path
	if ((size = line.find_first_of(' ', n_spaces)) == std::string::npos)
	{
		this->_ret = 400;
		std::cerr << RED << "No HTTP version" << RESET << std::endl;
		return 400;
	}
	this->_path.assign(line, n_spaces, size - n_spaces);
	return this->readVersion(line, size);
}


int					Request::readVersion(const std::string& line, size_t i)
{
	if ((i = line.find_first_not_of(' ', i)) == std::string::npos)
	{
		this->_ret = 400;
		std::cerr << RED << "No HTTP version" << RESET << std::endl;
		return 400;
	}
	if (line[i] == 'H' && line[i + 1] == 'T' && line[i + 2] == 'T' &&
			line[i + 3] == 'P' && line[i + 4] == '/')
		this->_version.assign(line, i + 5, 3);
	if (this->_version != "1.0" && this->_version != "1.1") // solo soportamos HTTP/1.1 o tambien HTTP/1.0
	{
		this->_ret = 400;
		std::cerr << RED << "BAD HTTP VERSION (" << this->_version << ")" << RESET << std::endl;
		return (this->_ret);
	}
	return (this->checkMethod());
}

/**
 * Check if the method is valid
 */
int					Request::checkMethod()
{
	for (size_t i = 0; i < this->methods.size(); i++)
		if (this->methods[i] == this->_method)
			return this->_ret;
	std::cerr << RED << "Invalid method requested" << RESET << std::endl;
	this->_ret = 400;
	return this->_ret;
}

int					Request::checkPort()
{
	size_t i = this->_headers["Host"].find_first_of(':');

	if (i == std::string::npos)
		this->_port = 8080; // por defecto
	else
	{
		std::string tmp(this->_headers["Host"], i + 1);
		this->_port = ft_atoi(tmp.c_str());
	}
	return (this->_port);
}

void				Request::findQuery()
{
	size_t		i;

	i = this->_path.find_first_of('?');
	if (i != std::string::npos)
	{
		logDebug("[DEBUG] Query found in path");
		this->_query.assign(this->_path, i + 1, std::string::npos);
		this->_path = this->_path.substr(0, i);
	}
}

std::ostream&		operator<<(std::ostream& os, const Request& re)
{
	std::map<std::string, std::string>::const_iterator	it;

	os << "Method : " << re.getMethod() << " |\tHTTP version : ";
	os << re.getVersion() << '\n';
	os << "Port : " << re.getPort() << '\n';
	os << "Path : " << re.getPath() << '\n';

	for (it = re.getHeaders().begin(); it != re.getHeaders().end(); it++)
		os << it->first << ": " << it->second << '\n';

	os << '\n' << "Request body :\n" << re.getBody() << '\n';

	return os;
}
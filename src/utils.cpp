#include "../include/WebServ.hpp"

/**
 * Set the socket to non-blocking mode.
 * This is useful for handling multiple connections without blocking.
 * TODO: is this really necessary when using select?
 */
void set_nonblocking(int sock) {
	int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
		logInfo(
			"Failed to get flags for socket %d: %s",
			sock,
			strerror(errno)
        );
		return;
	}
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl set non-blocking");
	}
}

std::string& pop(std::string& str)
{
	if (str.size())
		str.resize(str.size() - 1);
	return str;
}

std::string	readKey(const std::string& line)
{
	std::string	ret;

	size_t	i = line.find_first_of(':');
	ret.append(line, 0 , i);
	capitalize(ret);
	return (strip(ret, ' '));
}

std::string&	capitalize(std::string& str)
{
	size_t	i = 0;

	to_lower(str);
	str[i] = std::toupper(str[i]);
	while((i = str.find_first_of('-', i + 1)) != std::string::npos)
	{
		if (i + 1 < str.size())
			str[i + 1] = std::toupper(str[i + 1]);
	}
	return str;
}

std::string&	to_upper(std::string& str)
{
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
	return str;
}

std::string&	to_lower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

std::string&	strip(std::string& str, char c)
{
	size_t	i;

	if (!str.size())
		return str;
	i = str.size();
	while (i && str[i - 1] == c)
		i--;
	str.resize(i);
	for (i = 0; str[i] == c; i++);
	str = str.substr(i, std::string::npos);
	return str;
}

std::string	to_string(size_t n)
{
	std::stringstream tmp;

	tmp << n;

	return tmp.str();
}

std::string	readValue(const std::string& line)
{
	size_t i;
	std::string	ret;

	i = line.find_first_of(':');
	i = line.find_first_not_of(' ', i + 1);
	if (i != std::string::npos)
		ret.append(line, i, std::string::npos);
	return (strip(ret, ' '));
}

std::vector<std::string>	split(const std::string& str, char c)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, c))
		tokens.push_back(token);
	return tokens;
}

int	ft_atoi(const char *str)
{
	int	i;
	int	sign;
	int	num;

	i = 0;
	sign = 1;
	num = 0;
	ft_skip_spacenl(str, &i);
	if (str[i] == '-')
		sign = -1;
	if (str[i] == '-' || str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
		num = num * 10 + (str[i++] - '0');
	return (num * sign);
}

void	ft_skip_spacenl(const char *str, int *i)
{
	while ((str[*i] == ' ' || str[*i] == '\t' || str[*i] == '\n')
	|| (str[*i] == '\r' || str[*i] == '\v' || str[*i] == '\f'))
		(*i)++;
}

// This function compares two language pairs based on their float values.
// It is used to sort languages by their value.
bool compare_langs(const std::pair<std::string, float> first, const std::pair<std::string, float> second)
{
  return ( first.second > second.second );
}

int     ft_stoi(std::string str)
{
	std::stringstream ss(str);
	if (str.length() > 10)
		throw std::exception();
	for (size_t i = 0; i < str.length(); ++i)
	{
		if(!isdigit(str[i]))
			throw std::exception();
	}
	int res;
	ss >> res;
	return (res);
}

/**
 * check if 'word' is a substring of 'str'
 */
bool in_str(const std::string &word, const std::string &str) {
	return str.find(word) != std::string::npos;
}
/**
 * This works for any kind of file: text, binary, images, pdfs, etc.
 */
std::string read_file_binary(const std::string &file_path) {
	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file) {
		logError("No se pudo abrir el archivo %s.", file_path.c_str());
		throw HttpException(HttpStatusCode::Forbidden);; // TODO: what? que error va aqui?
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

/**
 * This works only for text files.
 */
std::string read_file_text(const std::string &file_path) {
	std::ifstream file(file_path.c_str());
	if (!file) {
		logError("No se pudo abrir el archivo %s.", file_path.c_str());
		throw HttpException(HttpStatusCode::Forbidden);; // TODO: what? que error va aqui?
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string replace_all(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return result;
}

/**
 * check if 'path' matches 'prefix' exactly or as a directory
 * los directorios deben coincidir y el path puede tener o no una barra al final
 * Ejemplos:
 * path /images matches prefix /images and /images/	
 * path /img does not match prefix /images
 * path /images/pic.jpg matches prefix /images/
 */
bool path_matches(const std::string& prefix, const std::string& path) {
    // Normaliza: quita barra final de prefix y path si la tienen (excepto si son solo "/")
    std::string norm_prefix = prefix;
    std::string norm_path = path;
	if (prefix == "/") return true; // root matches everything
    if (norm_prefix.length() > 1 && norm_prefix[norm_prefix.length() - 1] == '/')
        norm_prefix.erase(norm_prefix.length() - 1);
    if (norm_path.length() > 1 && norm_path[norm_path.length() - 1] == '/')
        norm_path.erase(norm_path.length() - 1);
    // Coincidencia exacta
    if (norm_path == norm_prefix)
        return true;
    // path es subdirectorio o archivo dentro de prefix
    if (norm_path.size() > norm_prefix.size() && norm_path.compare(0, norm_prefix.size(), norm_prefix) == 0 && norm_path[norm_prefix.size()] == '/')
        return true;
    return false;
}

std::string method_toString(int method) {
	switch (method) {
		case M_GET: return "GET";
		case M_POST: return "POST";
		case M_DELETE: return "DELETE";
		case M_PUT: return "PUT";
		case M_HEAD: return "HEAD";
		default: {
			throw std::runtime_error("Invalid method enum value");
			return "UNKNOWN";
		}
	}
}

short method_toEnum(const std::string& method) {
	if (method == "GET") return M_GET;
	else if (method == "POST") return M_POST;
	else if (method == "DELETE") return M_DELETE;
	else if (method == "PUT") return M_PUT;
	else if (method == "HEAD") return M_HEAD;
	else {
		throw std::runtime_error("Invalid method string: " + method);
		return -1; // never reached
	}
}

bool ci_equal(const std::string& a, const std::string& b) {
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); ++i) {
		if (tolower(a[i]) != tolower(b[i])) return false;
	}
	return true;
}

/**
 * Get the file extension from a given path.
 * Ex: "/path/to/file.txt" -> ".txt"
 */
std::string getFileExtension(const std::string &path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos)
        return "";
    return path.substr(pos);
}
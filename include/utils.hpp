#ifndef UTILS_HPP
#define UTILS_HPP

#include "../include/WebServ.hpp"

enum e_methods {
	M_GET = 0,
	M_POST,
	M_DELETE,
	M_PUT,
	M_HEAD
};

int 		                ft_stoi(std::string str);
void                        set_nonblocking(int sock);
std::string&                pop(std::string& str);
std::string					readKey(const std::string& line);
std::string&				capitalize(std::string& str);
std::string&				to_upper(std::string& str);
std::string&				to_lower(std::string& str);
std::string&				strip(std::string& str, char c);
std::string					to_string(size_t n);
std::string					readValue(const std::string& line);
std::vector<std::string>	split(const std::string& str, char c);
void	                    ft_skip_spacenl(const char *str, int *i);
int		                    ft_atoi(const char *str);
bool                        compare_langs(const std::pair<std::string, float> first, const std::pair<std::string, float> second);
int 			ft_stoi(std::string str);
bool 			in_str(const std::string &word, const std::string &str);
std::string 	read_file_binary(const std::string &file_path);
std::string		read_file_text(const std::string &file_path);
std::string		replace_all(const std::string& str, const std::string& from, const std::string& to);
bool			path_matches(const std::string& prefix, const std::string& path);
std::string		method_toString(int method);
short			method_toEnum(const std::string& method);
bool			ci_equal(const std::string& a, const std::string& b);
std::string		getFileExtension(const std::string &path);

#endif // UTILS_HPP


#pragma once

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <string> 
#include <unistd.h>
#include <dirent.h>
#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <utility>

#include <cstdlib> 
#include <fstream>
#include <sstream>
#include <cctype>
#include <ctime>
#include <cstdarg>
#include <cassert>

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>
#include <list>

# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
# include <signal.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>

#include "paths.hpp"
#include "logging.hpp"
#include "statusCode.hpp"
#include "ServerUnit.hpp"
#include "Location.hpp"
#include "ConfigFile.hpp"
#include "ReadConfig.hpp"
#include "utils.hpp"
#include "Request.hpp"
#include "HttpResponse.hpp"
#include "Cgi.hpp"
#include "ServerManager.hpp"
#include "Cgi.hpp"


# define SUCCESS    0
# define ERROR      1

# define MAX_CONTENT_LENGTH 30000000
# define USAGE "Usage: ./webserv [config_file]"
# define BUFFER_SIZE  1024
# define BACKLOG_SIZE 10

# define DEFAULT_CONFIG_FILE "config/default.config"


# endif
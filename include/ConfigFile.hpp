#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include "../include/WebServ.hpp"

enum e_file_type {
	F_NOT_EXIST = -1,
	F_REGULAR_FILE = 1,
	F_DIRECTORY = 2,
	F_OTHER = 3
};

class ConfigFile {
	private:
		std::string		_path;
		size_t			_size;

	public:
		ConfigFile();
		ConfigFile(std::string const path);
		~ConfigFile();

		static int 		getTypePath(std::string const path);
		static bool 	checkFile(std::string const path, int mode);
		std::string		readFile(std::string path);
		static bool 	isFileExistAndReadable(std::string const path, std::string const index);
		static bool 	isFileExistAndExecutable(std::string const path, std::string const executable);

		std::string 	getPath();
		int 			getSize();
};

#endif
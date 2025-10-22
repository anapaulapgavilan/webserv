#include "../include/WebServ.hpp"

ConfigFile::ConfigFile() : _size(0) { }

ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0) { }

ConfigFile::~ConfigFile() { }

/**
 * ¿Es archivo (`1:F_REGULAR_FILE`), directorio (`2:F_DIRECTORY`) o inexistente (`-1:F_NOT_EXIST`)?
 */
int ConfigFile::getTypePath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (buffer.st_mode & S_IFREG) // regular file
			return (F_REGULAR_FILE);
		else if (buffer.st_mode & S_IFDIR) // directory
			return (F_DIRECTORY);
		else
			return (F_OTHER);
	}
	else
		return (F_NOT_EXIST);
}

/**
 * Comprueba permisos con `access()`
 * mode F_OK: Comprueba si el archivo existe.
 * mode R_OK: Comprueba si el archivo es legible (permiso de lectura).
 * mode X_OK: Comprueba si el archivo es ejecutable (permiso de ejecución).
 */
bool	ConfigFile::checkFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode) == 0);
}

bool ConfigFile::isFileExistAndReadable(std::string const path, std::string const index)
{
	// absolute path
	if (getTypePath(index) == F_REGULAR_FILE
		&& checkFile(index, R_OK))
		return (true);
	// relative path
	if (getTypePath(path + index) == F_REGULAR_FILE
		&& checkFile(path + index, R_OK))
		return (true);
	
	return (false);
}

bool ConfigFile::isFileExistAndExecutable(std::string const path, std::string const exec)
{
	// absolute path
	if (getTypePath(exec) == F_REGULAR_FILE && checkFile(exec, X_OK))
		return (true);
	// relative path
	if (getTypePath(path + exec) == F_REGULAR_FILE && checkFile(path + exec, X_OK))
		return (true);
	
	return (false);
}

/**
 * Lee el archivo de configuración.
 * Abre el archivo, lo lee y devuelve su contenido como `std::string`.
 */
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream config_file(path.c_str());
	if (!config_file || !config_file.is_open())
		return (NULL);

	std::stringstream stream_binding;
	stream_binding << config_file.rdbuf();
	return (stream_binding.str());
}

std::string ConfigFile::getPath()
{
	return (this->_path);
}

int ConfigFile::getSize()
{
	return (this->_size);
}

#include "Cgi.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iostream>

Cgi::Cgi(const std::string& scriptPath) : _scriptPath(scriptPath) {
    if (_scriptPath.empty()) {
        throw std::invalid_argument("Script path cannot be empty");
    }
}

Cgi::~Cgi() {}

void Cgi::setEnvVariables(const Request& req) {
    _envVariables.clear();
    _envVariables["REQUEST_METHOD"] = req.getMethod();
    _envVariables["QUERY_STRING"] = req.getQuery();

    if (req.getHeaders().count("Content-Type"))
        _envVariables["CONTENT_TYPE"] = req.getHeaders().at("Content-Type");
    if (req.getHeaders().count("Content-Length"))
        _envVariables["CONTENT_LENGTH"] = req.getHeaders().at("Content-Length");
    else {
        std::ostringstream oss;
        oss << req.getBody().size();
        _envVariables["CONTENT_LENGTH"] = oss.str();
    }

    _envVariables["SCRIPT_NAME"] = req.getPath();
    _envVariables["SERVER_PROTOCOL"] = req.getVersion();
}

/**
 * Executes the CGI script with the given request.
 * Returns the output of the script.
 * 
 * CGI:
 * - los datos relevantes (método, query string, content length, etc.) se pasan por variables de entorno
 * - el body (si existe) por la entrada estándar (stdin).
 * 
 * Se usa:
 * - stdin para enviar el body de la petición 
 * - stdout para recibir la salida del script.
 */
std::string Cgi::executeCgi(const Request& req) {
    std::string output;

    int stdin_pipe[2]; // Pipe for stdin
    int stdout_pipe[2]; // Pipe for stdout
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdin_pipe[0]); close(stdin_pipe[1]);
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        throw std::runtime_error("Fork failed");
    }

    if (pid == 0) { // Child process
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        // Setea variables de entorno
        setEnvVariables(req);
        char* const args[] = {const_cast<char*>(_scriptPath.c_str()), NULL};
        
        std::vector<std::string> env_strings;
        std::vector<char*> envp;
        for (std::map<std::string, std::string>::iterator it = _envVariables.begin(); it != _envVariables.end(); ++it) {
            env_strings.push_back(it->first + "=" + it->second);
        }
        for (size_t i = 0; i < env_strings.size(); ++i) {
            envp.push_back(const_cast<char*>(env_strings[i].c_str()));
        }

        // Prepare envp as array of char* terminated by NULL
        envp.push_back(NULL);
        execve(_scriptPath.c_str(), args, &envp[0]);
        logError("CGI execve failed: %s", strerror(errno));
        throw HttpException(HttpStatusCode::InternalServerError);
    } else {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        write(stdin_pipe[1], req.getBody().data(), req.getBody().size());
        close(stdin_pipe[1]); 

        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytesRead);
        }

        close(stdout_pipe[0]);
        waitpid(pid, NULL, 0);
    }
    return output;
}

std::string Cgi::run(const Request& req) {
    if (_scriptPath.empty()) {
        throw std::runtime_error("Script path is not set");
    }
    return executeCgi(req);
}
#include "Cgi.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>

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

    char inputTemplate[] = "/tmp/webserv_cgi_inXXXXXX";
    char outputTemplate[] = "/tmp/webserv_cgi_outXXXXXX";

    int input_fd = mkstemp(inputTemplate);
    if (input_fd == -1) {
        throw std::runtime_error("Failed to create CGI input temp file");
    }

    int output_fd = mkstemp(outputTemplate);
    if (output_fd == -1) {
        close(input_fd);
        unlink(inputTemplate);
        throw std::runtime_error("Failed to create CGI output temp file");
    }

    const std::string &body = req.getBody();
    size_t total_written = 0;
    while (total_written < body.size()) {
        ssize_t written = write(input_fd, body.data() + total_written, body.size() - total_written);
        if (written < 0) {
            if (errno == EINTR)
                continue;
            close(input_fd);
            close(output_fd);
            unlink(inputTemplate);
            unlink(outputTemplate);
            throw HttpException(HttpStatusCode::InternalServerError);
        }
        if (written == 0) {
            close(input_fd);
            close(output_fd);
            unlink(inputTemplate);
            unlink(outputTemplate);
            throw HttpException(HttpStatusCode::InternalServerError);
        }
        total_written += static_cast<size_t>(written);
    }
    if (lseek(input_fd, 0, SEEK_SET) == -1) {
        close(input_fd);
        close(output_fd);
        unlink(inputTemplate);
        unlink(outputTemplate);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(input_fd);
        close(output_fd);
        unlink(inputTemplate);
        unlink(outputTemplate);
        throw std::runtime_error("Fork failed");
    }

    if (pid == 0) { // Child process
        if (dup2(input_fd, STDIN_FILENO) == -1 || dup2(output_fd, STDOUT_FILENO) == -1) {
            logError("CGI dup2 failed: %s", strerror(errno));
            _exit(EXIT_FAILURE);
        }

        close(input_fd);
        close(output_fd);

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

        envp.push_back(NULL);
        execve(_scriptPath.c_str(), args, &envp[0]);
        logError("CGI execve failed: %s", strerror(errno));
        _exit(EXIT_FAILURE);
    }

    close(input_fd);
    close(output_fd);

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        unlink(inputTemplate);
        unlink(outputTemplate);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        unlink(inputTemplate);
        unlink(outputTemplate);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    std::ifstream outputFile(outputTemplate, std::ios::in | std::ios::binary);
    if (!outputFile) {
        unlink(inputTemplate);
        unlink(outputTemplate);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    std::ostringstream oss;
    oss << outputFile.rdbuf();
    output = oss.str();

    outputFile.close();
    unlink(inputTemplate);
    unlink(outputTemplate);
    return output;
}

std::string Cgi::run(const Request& req) {
    if (_scriptPath.empty()) {
        throw std::runtime_error("Script path is not set");
    }
    return executeCgi(req);
}
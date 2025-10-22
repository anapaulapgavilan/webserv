#include "../include/WebServ.hpp"

bool ServerManager::_running = true; // Initialize the static running variable

ClientRequest::ClientRequest()
        : buffer(""), max_size(0), current_size(0), content_length(-1), is_chunked(false),
            header_end(std::string::npos), body_start(0),
            request_path(""), method(""), headers_parsed(false) {}

void ClientRequest::append_to_buffer(const std::string& chunk) {
    buffer += chunk;
    current_size += chunk.size();
}

ServerManager::ServerManager()
  : _max_fd(0)
{
}

ServerManager::~ServerManager(){}

void ServerManager::setup(const std::vector<ServerUnit>& configs) {

    _servers = configs;
    logDebug("Setting up %zu server(s)", _servers.size());
    
    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        ServerUnit &server = _servers[i];
        bool reused = false;

        for (size_t j = 0; j < i; ++j) {
            const ServerUnit &prev = _servers[j];
            if (prev.getHost() == server.getHost() &&
                prev.getPort() == server.getPort())
            {
                server.setFd(prev.getFd());
                reused = true;
                break;
            }
        }

        if (!reused) {
            server.setUpIndividualServer();
        }

        _servers_map[server.getFd()] = server;

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server.getHost(), ipbuf, sizeof(ipbuf));
        logInfo(
            "Server %s bound to http://%s:%d, fd = %d",
            server.getServerName().c_str(),
            ipbuf,
            server.getPort(),
            server.getFd()
        );
    }
}
void ServerManager::_init_server_unit(ServerUnit server) {
    // listen
    int fd = server.getFd();
    if (listen(fd, BACKLOG_SIZE) < 0) {
        logInfo("listen(%d) failed: %s", fd, strerror(errno));
        exit(ERROR);
    }
    // Add to the read set
    FD_SET(fd, &_read_fds);
    // Update _max_fd
    if (fd > _max_fd) _max_fd = fd;
    logInfo("🐡 Server started on port %d", server.getPort());

}

int ServerManager::_get_client_server_fd(int client_socket) const {
    std::map<int, int>::const_iterator it = _client_server_map.find(client_socket);
    if (it == _client_server_map.end()) {
        return -1;
    }
    return it->second;
}


void ServerManager::init()
{
    _running = true;
    signal(SIGINT, ServerManager::_handle_signal); // Handle Ctrl+C

    FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);

    for (size_t i = 0; i < _servers.size(); ++i) 
        _init_server_unit(_servers[i]); // Initialize each server unit

    fd_set temp_read_fds;
    fd_set temp_write_fds;
    while (_running) {
        temp_read_fds = _read_fds;
        temp_write_fds = _write_fds;

        int activity = select(_max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) continue; // Interrupted by signal
            logInfo("Failed to select on sockets");
        }

        for (int fd = 0; fd <= _max_fd; ++fd) {
            if (FD_ISSET(fd, &temp_read_fds)) {
                if (_servers_map.find(fd) != _servers_map.end()) {
                    // The fd belongs to a server that has a new connection
                    _handle_new_connection(fd);
                } else {
                    // The fd belongs to a client that is sending data
                    _handle_read(fd);
                }
            }
            if (FD_ISSET(fd, &temp_write_fds)) {
                // The fd belongs to a client that is ready to write data
                _handle_write(fd);
            }
        }
    }
    logInfo("\nServers shutting down...");
}

void ServerManager::_handle_new_connection(int listening_socket) {
    int client_sock = accept(listening_socket, NULL, NULL);
    if (client_sock < 0) {
        logError("Failed to accept new connection on socket %d: %s", listening_socket, strerror(errno));
        return;
    }

    if (client_sock >= FD_SETSIZE) {
        logError("Too many open files, cannot accept new connection on socket %d", listening_socket);
        close(client_sock);
        return;
    }

    set_nonblocking(client_sock);
    FD_SET(client_sock, &_read_fds);
    if (client_sock > _max_fd) _max_fd = client_sock;

    _client_server_map[client_sock] = listening_socket; // Map client socket to server socket
    _read_requests[client_sock] = ClientRequest(); // Initialize Request object for the new client
    _write_buffer[client_sock] = ""; // Initialize write buffer for the new client
    _bytes_sent[client_sock] = 0; // Initialize bytes sent for the new client
    logInfo("🐠 New connection accepted on socket %d. Listening socket: %d", client_sock, listening_socket);
}

void ServerManager::_handle_write(int client_sock) {

    std::string remaining_response = _write_buffer[client_sock].substr(_bytes_sent[client_sock]);
    /*
    // Be defensive: if for any reason _bytes_sent is larger than buffer size,
    // clamp it to avoid std::out_of_range (which yields "basic_string" exceptions).
    size_t offset = 0;
    std::map<int, std::string>::iterator wb_it = _write_buffer.find(client_sock);
    if (wb_it != _write_buffer.end()) {
        size_t buf_size = wb_it->second.size();
        size_t sent = 0;
        std::map<int, size_t>::iterator bs_it = _bytes_sent.find(client_sock);
        if (bs_it != _bytes_sent.end()) sent = bs_it->second;
        offset = (sent > buf_size) ? buf_size : sent;
    }
    std::string remaining_response = _write_buffer[client_sock].substr(offset);
    */
    logInfo("🐠 Sending response to client socket %d", client_sock);
    size_t n = send(client_sock, remaining_response.c_str(), remaining_response.size(), 0);

    if (n <= 0) {
        _cleanup_client(client_sock);
        logError("Failed to send data to client socket %d: %s. Connection closed.", client_sock, strerror(errno));
        return;
    }
    _bytes_sent[client_sock] += n;
    if (_bytes_sent[client_sock] == _write_buffer[client_sock].size()) {
       if (_should_close_connection(_read_requests[client_sock].buffer, _write_buffer[client_sock])) {
            _cleanup_client(client_sock);
        } else {
            // Mantener la conexión: limpiar buffers y volver a modo lectura
            _read_requests[client_sock] = ClientRequest(); // Reset the Request object
            _write_buffer[client_sock].clear();
            _bytes_sent[client_sock] = 0;
            FD_CLR(client_sock, &_write_fds);
            FD_SET(client_sock, &_read_fds);
        }
    }
}
bool ServerManager::_should_close_connection(const std::string& request, const std::string& response) {
    // Check if the request or response contains "Connection: close"
    bool closing = false;
    if (in_str(request, "Connection: close")){
        logError("🐠 Closing connection (requested)");
        closing = true;
    } 
    if (in_str(response, "Connection: close")) {
        logError("🐠 Closing connection (response)");
        closing = true;
    }
    return closing;
}

bool ServerManager::parse_headers(int client_sock, ClientRequest &cr) {
    // Extraer Path y Content-Length si existe
    cr.header_end = cr.buffer.find("\r\n\r\n");
    if (cr.header_end == std::string::npos) return false; // faltan headers
    cr.body_start = cr.header_end + 4;

    // Primera línea. Request line: "GET /path HTTP/1.1"
    size_t line_end = cr.buffer.find("\r\n");
    if (line_end == std::string::npos) return false; // faltan headers
    std::string req_line = cr.buffer.substr(0, line_end);
    {
        // Método y path. 
        size_t sp1 = req_line.find(' ');
        size_t sp2 = (sp1 == std::string::npos) ? std::string::npos : req_line.find(' ', sp1 + 1);
        if (sp1 != std::string::npos && sp2 != std::string::npos) {
            cr.method = req_line.substr(0, sp1);
            cr.request_path = req_line.substr(sp1 + 1, sp2 - sp1 - 1);
        }
    }
    // Headers
    cr.content_length = -1;
    size_t pos = line_end + 2;
    while (pos < cr.header_end) {
        size_t next = cr.buffer.find("\r\n", pos);
        if (next == std::string::npos || next > cr.header_end) break;
        std::string line = cr.buffer.substr(pos, next - pos);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            strip(key, ' ');
            strip(val, ' ');
            if (ci_equal(key, "Content-Length")) {
                cr.content_length = strtol(val.c_str(), NULL, 10);
                if (cr.content_length < 0) cr.content_length = -1;
            }
            if (ci_equal(key, "Transfer-Encoding")) {
                std::string lower = val;
                to_lower(lower);
                if (lower.find("chunked") != std::string::npos) {
                    cr.is_chunked = true;
                }
            }
            // Si quieres, aquí puedes detectar "Transfer-Encoding: chunked" y rechazarlo.
        }
        pos = next + 2;
    }
    // Determinar max_size (location > server)
    int server_fd = _get_client_server_fd(client_sock);
    if (server_fd < 0) {
        logError("Could not find server for client socket %d", client_sock);
        return false;
    }
    ServerUnit &server = _servers_map[server_fd];
    const Location* loc = _find_best_location(cr.request_path, server.getLocations());
    if (loc)
        cr.max_size = loc->getMaxBodySize();
    else
        cr.max_size = server.getClientMaxBodySize();

    cr.headers_parsed = true;

    // Si hay Content-Length y supera el límite, corta ya (como Nginx)
    if (cr.max_size > 0 && cr.content_length >= 0
        && (size_t)cr.content_length > cr.max_size) {
        // Señal para el caller: devolverá 413
        logError("Payload declared too large: CL=%ld > limit=%zu",
                 cr.content_length, cr.max_size);
    }
    return true;
}

void ServerManager::_handle_read(int client_sock) {
    char buffer[BUFFER_SIZE];
    int n;

    logInfo("🐟 Client connected on socket %d", client_sock);
    ClientRequest &cr = _read_requests[client_sock];

    // receive client request data
    while ((n = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        cr.append_to_buffer(std::string(buffer, n));

        // Parsear headers una sola vez, cuando estén completos
        if (!cr.headers_parsed) {
            if (!parse_headers(client_sock, cr)) {
                // Aún no hay headers completos; sigue leyendo
                continue;
            }
            // Si Content-Length ya excede el límite → 413 inmediato
            if (cr.max_size > 0 && cr.content_length >= 0
                && (size_t)cr.content_length > cr.max_size) {
                _write_buffer[client_sock] =
                    "HTTP/1.1 413 Payload Too Large\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<h1>413 Payload Too Large</h1>";
                _bytes_sent[client_sock] = 0;
                FD_CLR(client_sock, &_read_fds);
                FD_SET(client_sock, &_write_fds);
                return;
            }
        }
        // Si ya hay headers, check max body size y completitud
        if (cr.headers_parsed) {
            size_t body_bytes = (cr.current_size > cr.body_start)
                                ? (cr.current_size - cr.body_start)
                                : 0;

            // check max body size
            if (cr.max_size > 0 && body_bytes > cr.max_size) {
                logError("Client %d exceeded max body size (body=%zu > %zu). 413.",
                         client_sock, body_bytes, cr.max_size);
                _write_buffer[client_sock] =
                    "HTTP/1.1 413 Payload Too Large\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<h1>413 Payload Too Large</h1>";
                _bytes_sent[client_sock] = 0;
                FD_CLR(client_sock, &_read_fds);
                FD_SET(client_sock, &_write_fds);
                return;
            }

            // check request completeness
            bool complete = false;
            if (cr.content_length < 0) {
                // No hay body esperado → con headers basta
                complete = true;
            } else {
                complete = (body_bytes >= (size_t)cr.content_length);
            }
            if (complete) {
                logInfo("🐠 Request complete from client socket %d", client_sock);
                // responder sin esperar a que el cliente cierre la conexión.
                _write_buffer[client_sock] = prepare_response(client_sock, cr.buffer);
                _bytes_sent[client_sock] = 0;
                FD_CLR(client_sock, &_read_fds);
                FD_SET(client_sock, &_write_fds);
                return;
            }
        }
    }
    if (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
        logError("Failed to receive data from client");
        _cleanup_client(client_sock);
        return;
    }

    if (n == 0) {
        // el cliente cerró la conexión.
        // Cierre del peer: decide con lo que tengas
        if (cr.headers_parsed) {
            size_t body_bytes = (cr.current_size > cr.body_start)
                                ? (cr.current_size - cr.body_start)
                                : 0;
            if (cr.content_length < 0 || body_bytes >= (size_t)cr.content_length) {
                logDebug("content_length: %ld, body_bytes: %zu", cr.content_length, body_bytes);
                logInfo("🐠 Request complete from client socket %d (on close)", client_sock);
                // last opportunity to respond
                _write_buffer[client_sock] = prepare_response(client_sock, cr.buffer);
            } else {
                logError("Client disconnected before sending full body on socket %d. 400.", client_sock);
                _write_buffer[client_sock] =
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<h1>400 Bad Request</h1>";
            }
        } else {
            logError("Client disconnected before sending headers on socket %d. 400.", client_sock);
            _write_buffer[client_sock] =
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n"
                "<h1>400 Bad Request</h1>";
        }
        _bytes_sent[client_sock] = 0;
        FD_CLR(client_sock, &_read_fds);
        FD_SET(client_sock, &_write_fds);
        return;
    }
}

bool ServerManager::_request_complete(const ClientRequest& clrequest) {
    const std::string& request = clrequest.buffer;
    size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false; // Headers incompletos
  
    int content_length = clrequest.content_length;
    if (clrequest.headers_parsed && content_length == 0)
        return true; // No hay body esperado

    size_t body_start = header_end + 4;
    size_t body_size = clrequest.current_size - body_start;

    return body_size >= (size_t)content_length;
}

// Check if buffer contains the terminating chunk ("0\r\n") for chunked bodies
bool buffer_has_final_chunk(const std::string &buffer) {
    // Search for "\r\n0\r\n" or start-of-buffer "0\r\n"
    if (buffer.find("\r\n0\r\n") != std::string::npos) return true;
    if (buffer.size() >= 3) {
        // also allow if buffer ends with "0\r\n" without leading CRLF
        size_t len = buffer.size();
        if (buffer.substr(len - 3) == "0\r\n") return true;
    }
    return false;
}

// Drain remaining body from socket for chunked or content-length requests.
// Returns true if successfully drained (or there was nothing to drain), false on error/timeout.
bool ServerManager::_drain_request_body(int client_sock, ClientRequest &cr) {
    char buf[BUFFER_SIZE];
    int n;
    // If we already have the terminating chunk in the buffer, nothing to read
    if (cr.is_chunked && buffer_has_final_chunk(cr.buffer))
        return true;

    // If content-length known: read until we have all bytes
    if (cr.content_length >= 0) {
        size_t body_start = cr.body_start;
        size_t have = (cr.current_size > body_start) ? (cr.current_size - body_start) : 0;
        long remaining = cr.content_length - (long)have;
        while (remaining > 0) {
            n = recv(client_sock, buf, sizeof(buf), 0);
            if (n > 0) {
                remaining -= n;
            } else if (n == 0) {
                // client closed early
                return false;
            } else {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // no more data now; give up
                    return false;
                }
                return false;
            }
        }
        return true;
    }

    // If chunked: read until we see the terminating chunk 0\r\n
    if (cr.is_chunked) {
        std::string tmp;
        while (true) {
            n = recv(client_sock, buf, sizeof(buf), 0);
            if (n > 0) {
                tmp.append(buf, n);
                if (buffer_has_final_chunk(tmp)) return true;
                // continue reading
                continue;
            } else if (n == 0) {
                // client closed; check if tmp had final chunk
                return buffer_has_final_chunk(tmp);
            } else {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // no more data now
                    return false;
                }
                return false;
            }
        }
    }
    // Nothing to drain
    return true;
}
/**
 * If the request has a body (chunked or content-length), try to read and discard it.
 * Returns true if the body was fully drained (or there was no body to drain).
 * Returns false if draining failed (error, timeout, incomplete).
 * If true is returned, the response string is adjusted to remove "Connection: close"
 * so that the connection can be reused.
 */
bool ServerManager::_try_drain_and_adjust_response(int client_socket, std::string &response_str) {
    // get ClientRequest
    std::map<int, ClientRequest>::iterator it = _read_requests.find(client_socket);
    if (it == _read_requests.end()) return false;
    ClientRequest &cr = it->second;
    // If there is no body to drain, nothing to do
    if (!(cr.is_chunked || cr.content_length >= 0)) return true;

    logDebug("Attempting to drain request body for client %d", client_socket);
    bool drained = _drain_request_body(client_socket, cr);
    if (!drained) {
        logDebug("Draining failed for client %d; will close connection after response", client_socket);
        return false;
    }
    // We consumed it: reset request state and remove Connection: close from response
    cr.buffer.clear(); cr.current_size = 0; cr.header_end = std::string::npos;
    cr.body_start = 0; cr.content_length = -1; cr.headers_parsed = false; cr.is_chunked = false;
    size_t pos = response_str.find("Connection: close");
    if (pos != std::string::npos) {
        response_str.erase(pos, strlen("Connection: close"));
    }
    return true;
}

const Location* ServerManager::_find_best_location(const std::string& request_path, const std::vector<Location> &locations) const{
    const Location* best_match = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        const Location& loc = locations[i];

        std::string loc_path = loc.getPathLocation();
        if (path_matches(loc_path, request_path)) {
            // Preferimos el que tenga el prefix MÁS LARGO
            if (loc_path.size() > best_len) {
                best_match = &loc;
                best_len = loc_path.size();
            }
        }
    }
    return best_match; // puede ser NULL si ninguno matchea
}
void ServerManager::_apply_location_config(
    const Location *loc,
    std::string &root,
    std::string &index,
    bool &autoindex,
    std::string &full_path,
    const std::string &request_path,
    const std::string &request_method,
    bool &used_alias
) {
    if (!loc)
        return;

    logDebug("🍍 Location matched: %s", loc->getPathLocation().c_str());

    // Verificar métodos permitidos
    if (!loc->getMethods()[method_toEnum(request_method)]) {
        std::string methods = loc->getPrintMethods();
        throw HttpExceptionNotAllowed(methods);
    }
    
    // Root
    if (!loc->getRootLocation().empty())
        root = loc->getRootLocation();

    // Alias
    if (!loc->getAlias().empty()) { // ONLY ABSOLUTE ALIAS SUPPORTED
        full_path = loc->getAlias() + request_path.substr(loc->getPathLocation().size());
        used_alias = true;
        logDebug("🍍 Using alias: %s -> %s", request_path.c_str(), full_path.c_str());
    }

    // Index
    if (!loc->getIndexLocation().empty())
        index = loc->getIndexLocation();

    // Autoindex
    if (loc->getAutoindex() != autoindex)
        autoindex = loc->getAutoindex();
}

void ServerManager::_handle_directory_case(
    std::string &full_path,
    const std::string &request_path,
    const std::string &index,
    bool autoindex,
    Request &request
) {
    if (ConfigFile::getTypePath(full_path) != F_DIRECTORY)
        return;
    logDebug("🍍 Handling directory case for path: %s", full_path.c_str());

    // without trailing slash -> redirect
    if (!request_path.empty() && request_path[request_path.size() - 1] != '/') {
        std::string new_location = request_path + "/";
        throw HttpExceptionRedirect(HttpStatusCode::MovedPermanently, new_location);
    }

    // index defined and exists -> use it
    if (!index.empty()) { // index defined
        std::string index_path = full_path + index;
        if (ConfigFile::getTypePath(index_path) == F_REGULAR_FILE) { 
            full_path = index_path;
            logDebug("🍍 Directory index found: %s", index_path.c_str());
            return;
        }
    }
    // no index defined or not found
    if (autoindex) {
        // autoindex ON
        logDebug("🍍 Autoindex enabled for directory %s", full_path.c_str());
        request.setAutoindex(true);
    }
    else {
        // No hay index y autoindex está deshabilitado → 404
        throw HttpException(HttpStatusCode::NotFound);
    }
    
}

void ServerManager::_apply_redirection(const Location *loc) {
    if (loc && !loc->getReturn().empty()) {
        std::string new_location = loc->getReturn();
        throw HttpExceptionRedirect(HttpStatusCode::MovedPermanently, new_location);
    }
    // else if (server.hasReturn()) {...}

    /*
    NGINX does:

    location /old {
        return 301 /new;
    }
    location /other_old {
        return 302 http://example.com/other_new;
    }

    --> o sea, con codigo de redireccion 
    ademas!! acepta return en server block y nosotros no.
    */
}

/**
 * Resolve the request path based on server configuration.
 * index, root, alias, return, etc.
 */
void ServerManager::resolve_path(Request &request, int client_socket) {
    int server_fd = _get_client_server_fd(client_socket);
    if (server_fd == -1) {
        logError("resolve_path: client_socket %d not found in _client_server_map!", client_socket);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    ServerUnit &server = _servers_map[server_fd];
    std::string path = request.getPath();
    path = path_normalization(clean_path(path));
    
    // server block config
    std::string root = server.getRoot();
    std::string index = server.getIndex();
    bool autoindex = server.getAutoindex();
    bool used_alias = false;
    std::string full_path;
    
    // 1. search best location and apply
    const Location *loc = _find_best_location(path, server.getLocations());
    _apply_redirection(loc);
    _apply_location_config(loc, root, index, autoindex, full_path, path, request.getMethod(), used_alias);
    request.setMatchedLocation(loc);

    if (!used_alias)
        full_path = root + path;

    // 4. Gestionar directorios, autoindex e index
    _handle_directory_case(full_path, path, index, autoindex, request);

    // checking the file existence is done by the methods resolver.
    
    // finally, set the resolved path
    request.setPath(full_path);
}

std::string ServerManager::prepare_response(int client_socket, const std::string &request_str) {
    std::string response_str;

    try {
        logDebug("\n----------\n⛺️Parsing request:\n%s", request_str.c_str());
        Request request(request_str);
        if (request.getRet() != 200) 
            throw HttpException(request.getRet());
        logDebug("🍅 Request parsed. Query: [%s:%s]",request.getMethod().c_str(),request.getPath().c_str());
        resolve_path(request, client_socket);
        logDebug("🍅 preparing response. client socket: %i. Query: %s %s",
            client_socket, request.getMethod().c_str(), request.getPath().c_str());
        HttpResponse response(&request);
        response_str = response.getResponse();
        logInfo("response_str ok");
    } catch (const HttpExceptionRedirect &e) {
        int code = e.getStatusCode();
        std::string location = e.getLocation();
        logInfo("🍊 Acción: Redirigir con código %d a %s", code, location.c_str());
        HttpResponse response(code, location);
        response_str = response.getResponse();
        logInfo("response_str redirect ok");
    } catch (const HttpExceptionNotAllowed &e) {
        int code = e.getStatusCode();
        std::string methods = e.getAllowedMethods();
        logInfo("🍊 Acción: Método no permitido. Allowed: %s", methods.c_str());
        HttpResponse response(code);
        response.set_allow_methods(methods);
        response_str = response.getResponse();
        logInfo("response_str not allowed ok");
        logDebug("response:\n%s\n-----", response_str.c_str());
        // encapsulate drain-and-adjust logic in helper
        _try_drain_and_adjust_response(client_socket, response_str);
    } catch (const HttpException &e) {
        int code = e.getStatusCode();
        logError("HTTP Exception caught: %s, code %d", e.what(), code);
        response_str = prepare_error_response(client_socket, code);
        logInfo("response_str error ok");
        logDebug("response:\n%s\n-----", response_str.c_str());

    } catch (const std::exception &e) {
        // raise exc?
        logError("Exception: %s", e.what());
        //int code = HttpStatusCode::InternalServerError; // Default to 500 Internal Server
        exit(1);
    }
    logInfo("Done\n----------");
    return response_str;
}

std::string ServerManager::prepare_error_response(int client_socket, int code) {
    logInfo("Prep error: client socket %i. error %d", client_socket, code);
    std::string response_str;
    // first: try error page in config
    int server_fd = _get_client_server_fd(client_socket);
    if (server_fd == -1) {
        // no deberia pasar
        logError("prep error: client_socket %d not found in _client_server_map!", client_socket);
        HttpResponse response(HttpStatusCode::InternalServerError);
        return response.getResponse();
    }
    ServerUnit &server = _servers_map[server_fd];
    std::string err_page_path = server.getPathErrorPage(code);
    if (!err_page_path.empty()) {
        logInfo("🍊 Acción: Mostrar página de error %d desde %s", code, err_page_path.c_str());
        HttpResponse response(code, WWW_ROOT + err_page_path);
        response_str = response.getResponse();
        return response_str;
    }
    logDebug("prep error: error page for code %d not found in server config", code);
    // if not found, treat web server error
    std::string message = statusCodeString(code);
    switch (code) {
        case HttpStatusCode::NotFound:
        case HttpStatusCode::Forbidden:
        case HttpStatusCode::MethodNotAllowed:
            {
                // show error page
                logError("🍊 Acción: Mostrar página de error %d.", code);
                HttpResponse response(code);
                response_str = response.getResponse();
            }
            break;
        case HttpStatusCode::InternalServerError:
            logError("Error. %s. Acción: Revisar los registros del servidor.", message.c_str());
            exit(2);
            break;
        case HttpStatusCode::BadRequest:
            response_str =
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<h1>400 Bad Request</h1>";
            break;
        default:
            {
                logError("Error no gestionado: %s. hacer algo!.", message.c_str());
                // show error page
                logError("🍊 Acción: Mostrar página de error %d.", code);
                HttpResponse response(code);
                response_str = response.getResponse();
            }
            break;
    }
    return response_str;
}

void ServerManager::_cleanup_client(int client_sock) {
    FD_CLR(client_sock, &_read_fds);
    FD_CLR(client_sock, &_write_fds);
    close(client_sock);
    _client_server_map.erase(client_sock);
    _read_requests.erase(client_sock);
    _write_buffer.erase(client_sock);
    _bytes_sent.erase(client_sock);
    logInfo("🐟 Client socket %d cleaned up", client_sock);
}

void ServerManager::_handle_signal(int signal) {
	(void)signal;
    ServerManager::_running = false;
}

#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class ServerUnit;

struct ClientRequest {
    std::string buffer;       // headers + body acumulado
    size_t      max_size;     // límite efectivo (location o server); 0 = ilimitado
    size_t      current_size; // bytes totales recibidos (headers + body)
    long        content_length; // -1 si no hay Content-Length
    bool        is_chunked;    // true si Transfer-Encoding: chunked
    size_t      header_end;   // posición de "\r\n\r\n" (fin de headers) en buffer
    size_t      body_start;   // header_end + 4
    std::string request_path; // para elegir location
    std::string method;       // GET/POST/DELETE...
    bool        headers_parsed;

    ClientRequest();
    void append_to_buffer(const std::string& str);
};

class ServerManager {
    private:
        std::vector<ServerUnit>    _servers;
        std::map<int,ServerUnit>   _servers_map;
        std::map<int, int>          _client_server_map; // Buffer for incoming requests


        static bool _running;
        // select sets
        fd_set _read_fds;
        fd_set _write_fds;
        int _max_fd; // Maximum file descriptor for select

        // Buffers
        std::map<int, ClientRequest> _read_requests;
        std::map<int, std::string> _write_buffer;
        std::map<int, size_t> _bytes_sent;


        ServerManager(const ServerManager &other);
        ServerManager &operator=(const ServerManager &other);

        void _init_server_unit(ServerUnit server);
        int _get_client_server_fd(int client_socket) const;
        bool parse_headers(int client_sock, ClientRequest &cr);
        bool _try_drain_and_adjust_response(int client_socket, std::string &response_str);
        bool _drain_request_body(int client_sock, ClientRequest &cr);

        
        void resolve_path(Request &request, int client_socket);
        const Location* _find_best_location(const std::string& request_path, const std::vector<Location> &locations) const;
        void _apply_location_config(
            const Location *loc,
            std::string &root,
            std::string &index,
            bool &autoindex,
            std::string &full_path,
            const std::string &request_path,
            const std::string &request_method,
            bool &used_alias
        );
        void _handle_directory_case(
            std::string &full_path,
            const std::string &request_path,
            const std::string &index,
            bool autoindex,
            Request &request
        );
        void _apply_redirection(const Location *loc);


        std::string prepare_response(int client_socket, const std::string& request);
        std::string prepare_error_response(int client_socket, int code);
        
        
        static void _handle_signal(int signal);
        void _handle_new_connection(int listening_socket);
        void _handle_read(int client_sock);
        void _handle_write(int client_sock);
        void _cleanup_client(int client_sock);
        bool _request_complete(const ClientRequest& clrequest);
        bool _should_close_connection(const std::string& request, const std::string& response);

    public:
        ServerManager();
        ~ServerManager();

        void setup(const std::vector<ServerUnit>& servers);
        void init();
};

#endif

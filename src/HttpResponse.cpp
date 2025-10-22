#include "../include/WebServ.hpp"

const std::string HttpResponse::CRLF = "\r\n";
const std::string HttpResponse::version = "HTTP/1.1";

ResponseStatus::ResponseStatus()
        : code(0), message("Empty") {}
ResponseStatus::ResponseStatus(int code)
        : code(code), message(statusCodeString(code)) {}

void HttpResponse::reset_all() {
    _status_line = ResponseStatus(0);
    _body = "";
    _headers.content_type = "";
    _headers.content_length = "0";
    _headers.allow = "";
    _headers.connection = "";
    _headers.location = "";
}

HttpResponse::HttpResponse(Request *request) : _request(request) {
  reset_all();
  assert(request != NULL);
  if (request->getMethod() == "GET") 
    handle_GET();
  else if (request->getMethod() == "POST") {
    handle_POST();
  }
  else if (request->getMethod() == "DELETE" ) {
    if(request->getQuery().empty()) {
      set_empty_response_close(HttpStatusCode::NotFound);
    } else {
      std::cout << "[DEBUG] path: " << request->getPath() << std::endl;
      Cgi cgi("cgi-bin/deleteFile.py");
      std::string cgi_output = cgi.run(*request);
      set_empty_response_alive(HttpStatusCode::OK);
    }
  }
  
  if (_status_line.code == 0) {
    throw HttpException(HttpStatusCode::NotImplemented);
  }

}

std::string get_default_error_page(int errorCode) {
  std::string errorMessage = statusCodeString(errorCode);
  
  std::stringstream htmlResponse;
  htmlResponse << "<!DOCTYPE html>"
                << "<html lang=\"en\">"
                << "<head>"
                << "<meta charset=\"UTF-8\">"
                << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                << "<title>Error - " << errorCode << "</title>"
                << "<style>"
                << "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; color: #333; background-color: #f8f8f8; }"
                << ".error { font-size: 48px; font-weight: bold; color: #e74c3c; }"
                << ".message { font-size: 24px; color: #555; }"
                << "</style>"
                << "</head>"
                << "<body>"
                << "<div class=\"error\">" << errorCode << " - " << errorMessage << "</div>"
                << "<div class=\"message\">"

                // Puedes agregar un mensaje personalizado para cada error
                << "We are sorry."
                << "</div>"
                << "</body>"
                << "</html>";
  return htmlResponse.str();
}

/** creates default error page */
HttpResponse::HttpResponse(int errorCode) : _request(NULL) {
  _status_line = ResponseStatus(errorCode);
  _body = get_default_error_page(errorCode);

  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  // a ser pagina de error, cerrar conexion
  _headers.connection = "close";

}

/**
 * Manages error pages and redirections
  * If errorCode is a redirection (3xx), errorpage_or_location is the Location
  * If errorCode is an error (4xx, 5xx), errorpage_or_location is the path to the error page
  * If the error page does not exist, a default error page is generated
 */
HttpResponse::HttpResponse(int errorCode, const std::string &errorpage_or_location) : _request(NULL) {
  if (errorCode >= 301 && errorCode <= 308) {
    set_redirect_response(errorCode, errorpage_or_location);
    return;
  }
  
  _status_line = ResponseStatus(errorCode);
  std::string valid_path = errorpage_or_location;
  _body = read_file_binary(valid_path);

  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";
}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::getStatusLine() const {
  return version + " " + to_string(_status_line.code) + " " + _status_line.message;
}
std::string HttpResponse::getHeaders() const {
    std::string headers =
        "Content-Type: " + _headers.content_type + "\r\n" +
        "Content-Length: " + _headers.content_length + "\r\n";
    if (!_headers.allow.empty())
        headers += "Allow: " + _headers.allow + "\r\n";
    if (!_headers.location.empty())
        headers += "Location: " + _headers.location + "\r\n";
    headers += "Connection: " + _headers.connection + "\r\n";
    return headers;
}
std::string HttpResponse::getBody() const {
  return _body;
}
std::string HttpResponse::toString() const {
  return getStatusLine() + "\r\n" +
       getHeaders() + "\r\n" +
       getBody();
}

/**
 * checks if the file exists, otherwise throw 404
 */
std::string validate_path(const std::string &path) {
  std::ifstream file(path.c_str());
  if (!file.is_open()) {
    logError("File not found: %s", path.c_str());
    throw HttpException(HttpStatusCode::NotFound);
  }
  return path;
}

std::string discover_content_type(const std::string &filename) {
  std::string content_type;

  if (in_str(".css", filename))
    content_type = "text/css";
  else if (in_str(".js", filename))
    content_type = "application/javascript";
  else if (in_str(".jpg", filename) || in_str(".jpeg", filename))
    content_type = "image/jpeg";
  else if (in_str(".png", filename))
    content_type = "image/png";
  else 
    content_type = "text/html";

  return content_type;
}

void HttpResponse::handle_GET() {
  if (_request->getAutoindex()) {
    generate_autoindex(*_request);
    return;
  }

  std::string file_path = validate_path(_request->getPath());
  if (file_path == "www/photo-detail.html") {
    std::string html = read_file_text("www/photo-detail.html");

    Cgi cgi("cgi-bin/getFile.py");
    std::string photo_block = cgi.run(*_request);
    html = replace_all(html, "<!--PHOTO_DETAIL-->", photo_block);

    _body = html;
    _headers.content_type = "text/html";
    _headers.content_length = to_string(_body.size());
    _headers.connection = "keep-alive";

    int code = HttpStatusCode::OK;
    _status_line = ResponseStatus(code);
    return;
  }
  if (file_path == DEFAULT_INDEX) {
    generate_webindex(*_request);
    return;
  }
  // else
  _body = read_file_binary(file_path);

  _headers.content_type = discover_content_type(file_path);
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";

  int code = HttpStatusCode::OK;
  _status_line = ResponseStatus(code);
  
}

void HttpResponse::handle_POST() {
  const Location* loc = _request->getMatchedLocation();
  if (loc) {
    std::string ext = getFileExtension(_request->getPath());
    std::string cgiExec = loc->getCgiHandler(ext);
    if (!cgiExec.empty()) {
      // Ejecutar CGI
      Cgi cgi(cgiExec);
      std::string cgi_output = cgi.run(*_request);
      set_empty_response_alive(HttpStatusCode::Created);
      return;
    }
    // else {
    //   logError("location: %s has no CGI handler for extension %s", loc->getPathLocation().c_str(), ext.c_str());
    //   throw HttpException(HttpStatusCode::NotFound);
    // }
  }
  if  (_request->getPath() == UPLOADS_URI) {
      if (_request->getBody().empty()) {
        throw HttpException(HttpStatusCode::BadRequest);
      }
      logDebug("[DEBUG] Body size: %i", _request->getBody().size());
      Cgi cgi("cgi-bin/saveFile.py");
      std::string cgi_output = cgi.run(*_request);
      logDebug("[DEBUG] 🎾 CGI output: %s", cgi_output.c_str());
      set_empty_response_alive(HttpStatusCode::Created);
  } else {
    _status_line = ResponseStatus(HttpStatusCode::OK);
    _body = "";
    _headers.content_type = "text/html";
    _headers.content_length = to_string(_body.size());
    _headers.connection = "keep-alive";
  }
}

std::string HttpResponse::getResponse() const {
  return toString();
}

void HttpResponse::generate_autoindex(const Request& request) {
  logDebug("🍍 Generating autoindex for path: %s", request.getPath().c_str());
  std::string path = request.getPath();
  std::vector<std::string> entries;
  DIR *dir = opendir((path).c_str());
  if (!dir)
      throw HttpException(HttpStatusCode::InternalServerError);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
      if (std::string(entry->d_name) == ".") continue;
      entries.push_back(entry->d_name);
  }
  closedir(dir);

  std::ostringstream html;
  html << "<html><body><h1>Index" << "</h1><ul>";
  for (size_t i = 0; i < entries.size(); ++i)
      html << "<li><a href='" << entries[i] << "'>" << entries[i] << "</a></li>";
  html << "</ul></body></html>";

  _body = html.str();
  _headers.content_type = "text/html";
  _headers.content_length = to_string(_body.size());
  _headers.connection = "keep-alive";

  int code = HttpStatusCode::OK;
  _status_line = ResponseStatus(code);
}

void HttpResponse::generate_webindex(const Request& request) {
    assert(request.getPath() == DEFAULT_INDEX);
  
    std::ifstream file(request.getPath().c_str());
    if (!file.is_open()) {
      throw HttpException(HttpStatusCode::NotFound);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html = buffer.str();

    Cgi cgi("cgi-bin/getIndex.py");
    std::string galeria = cgi.run(request); // Puedes pasar un Request vacío o uno simulado

    size_t pos = html.find("<!--GALERIA-->");
    if (pos != std::string::npos)
        html.replace(pos, std::string("<!--GALERIA-->").length(), galeria);

    _body = html;

    _headers.content_type = "text/html";
    _headers.content_length = to_string(html.size());
    _headers.connection = "keep-alive";

    int code = HttpStatusCode::OK;
    _status_line = ResponseStatus(code);
}

void HttpResponse::set_redirect_response(int code, const std::string& location) {
    assert (code >= 300 && code < 400);
    _status_line = ResponseStatus(code);
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "keep-alive";
    _headers.location = location;
    _body = ""; // Sin cuerpo
}

void HttpResponse::set_empty_response_alive(int code) {
    _status_line = ResponseStatus(code);
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "keep-alive";
    _body = ""; // Sin cuerpo
}

void HttpResponse::set_empty_response_close(int code) {
    _status_line = ResponseStatus(code);
    _headers.content_type = "text/html";
    _headers.content_length = "0";
    _headers.connection = "close";
    _body = ""; // Sin cuerpo
}

void HttpResponse::set_allow_methods(const std::string& methods) {
    _headers.allow = methods;
}



#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP

# include "../include/WebServ.hpp"


// CLASS HttpException ////////////////////////////////////////////////////////
class HttpException : public std::exception {
private:
    int statusCode;  // Código de estado HTTP
    std::string message; // Mensaje de error

public:
    // Constructor con código de estado y mensaje
    HttpException(int code);

    // Obtener el código de estado HTTP
    int getStatusCode() const;

    // Sobrescribir el método 'what()' para devolver el mensaje
    virtual const char* what() const throw() {
        return message.c_str();
    }
    virtual ~HttpException() throw() {}
    
};


std::string statusCodeString(int);

namespace HttpStatusCode {
    const int Continue = 100;
    const int SwitchingProtocol = 101;
    const int OK = 200;
    const int Created = 201;
    const int Accepted = 202;
    const int NonAuthoritativeInformation = 203;
    const int NoContent = 204;
    const int ResetContent = 205;
    const int PartialContent = 206;
    const int MultipleChoice = 300;
    const int MovedPermanently = 301;
    const int MovedTemporarily = 302;
    const int SeeOther = 303;
    const int NotModified = 304;
    const int TemporaryRedirect = 307;
    const int PermanentRedirect = 308;
    const int BadRequest = 400;
    const int Unauthorized = 401;
    const int Forbidden = 403;
    const int NotFound = 404;
    const int MethodNotAllowed = 405;
    const int NotAcceptable = 406;
    const int ProxyAuthenticationRequired = 407;
    const int RequestTimeout = 408;
    const int Conflict = 409;
    const int Gone = 410;
    const int LengthRequired = 411;
    const int PreconditionFailed = 412;
    const int PayloadTooLarge = 413;
    const int URITooLong = 414;
    const int UnsupportedMediaType = 415;
    const int RequestedRangeNotSatisfiable = 416;
    const int ExpectationFailed = 417;
    const int ImATeapot = 418;
    const int MisdirectedRequest = 421;
    const int TooEarly = 425;
    const int UpgradeRequired = 426;
    const int PreconditionRequired = 428;
    const int TooManyRequests = 429;
    const int RequestHeaderFieldsTooLarge = 431;
    const int UnavailableForLegalReasons = 451;
    const int InternalServerError = 500;
    const int NotImplemented = 501;
    const int BadGateway = 502;
    const int ServiceUnavailable = 503;
    const int GatewayTimeout = 504;
    const int HTTPVersionNotSupported = 505;
    const int VariantAlsoNegotiates = 506;
    const int InsufficientStorage = 507;
    const int NotExtended = 510;
    const int NetworkAuthenticationRequired = 511;
}

// CLASS HttpExceptionRedirect ////////////////////////////////////////////////////////

class HttpExceptionRedirect : public HttpException {
private:
    std::string newLocation; // Nueva ubicación para la redirección 
public:
    HttpExceptionRedirect(int code, const std::string& location)
        : HttpException(code), newLocation(location) {}
    
    const std::string& getLocation() const { return newLocation; }
    virtual ~HttpExceptionRedirect() throw() {}
};


// CLASS HttpExceptionNotAllowed ////////////////////////////////////////////////////////

class HttpExceptionNotAllowed : public HttpException {
private:
    std::string methods; // lista de métodos permitidos
public:
    HttpExceptionNotAllowed(const std::string& methods)
        : HttpException(HttpStatusCode::MethodNotAllowed), methods(methods) {}
    
    const std::string& getAllowedMethods() const { return methods; }
    virtual ~HttpExceptionNotAllowed() throw() {}
};

#endif

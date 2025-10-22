#include "../include/WebServ.hpp"

/**
 Add traces to logging
To add traces (file, line, function) to your error messages and exceptions in C++98, you can use the predefined macros:

__FILE__ — current file name
__LINE__ — current line number
__FUNCTION__ — current function name (supported by most compilers, including GCC)
*/

void _log_buffer(const char* level, const char* color, const char* buffer) {

    #if USE_DATE
    // Get current time
    time_t now = time(0);
    struct tm tstruct;
    char timebuf[80];
    tstruct = *localtime(&now);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d.%X", &tstruct);
    #endif

    #if USE_COLOR
    #if USE_DATE
    std::cout << color << "[" << timebuf << "] " << level << " " << buffer << RESET << "\n";
    #else
    std::cout << color << level << " " << buffer << RESET << "\n";
    #endif
    #else
    #if USE_DATE
    std::cout << "[" << timebuf << "] " << level << " " << buffer << "\n";
    #else
    std::cout << level << " " << buffer << "\n";
    #endif
    #endif
}

void logInfo(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    _log_buffer("INFO", GREEN, buffer);
}

void logError(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    _log_buffer("ERROR", RED, buffer);
    // std::cerr << RED << buffer << RESET << "\n";
}

void logDebug(const char* msg, ...)
{
    if (!DEBUG)
        return;

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    _log_buffer("DEBUG", BLUE, buffer);
}
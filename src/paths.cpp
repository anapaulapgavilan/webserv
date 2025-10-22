#include "../include/WebServ.hpp"

/**
 * Clean the path by replacing URL-encoded characters.
 * Currently replaces %20 with space.
 */
std::string clean_path(const std::string& path) {
    std::string cleaned = path;
    // Replace %20 with space
    cleaned = replace_all(cleaned, "%20", " ");
    // Add more replacements if needed
    return cleaned;
}

/**
 * Normalize the path by resolving ., .. and redundant slashes.
 * To prevent directory traversal attacks.
 */
std::string path_normalization(const std::string& path) {
    std::string normalized = path;
    // Replace multiple slashes with a single slash
    size_t pos;
    while ((pos = normalized.find("//")) != std::string::npos) {
        normalized.replace(pos, 2, "/");
    }
    // Remove occurrences of "/./"
    while ((pos = normalized.find("/./")) != std::string::npos) {
        normalized.replace(pos, 3, "/");
    }
    // Handle "/../" by removing the preceding directory
    while ((pos = normalized.find("/../")) != std::string::npos) {
        if (pos == 0) {
            normalized.erase(0, 3); // Remove leading "/.."
        } else {
            size_t prev_slash = normalized.rfind('/', pos - 1);
            if (prev_slash == std::string::npos) {
                normalized.erase(0, pos + 4); // Remove everything before "/../"
            } else {
                normalized.erase(prev_slash, pos + 3 - prev_slash);
            }
        }
    }
    return normalized;
}
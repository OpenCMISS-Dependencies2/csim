#ifndef UTILS_H
#define UTILS_H

#include <string>

/**
 * Retrieve the contents of the resource located at the given URL.
 * @param url The location of the resource to retrieve.
 * @return The content of the resource located at the given URL; or an empty string if unable
 * to retrieve the content.
 */
std::string getUrlContent(const std::string& url);

#endif // UTILS_H

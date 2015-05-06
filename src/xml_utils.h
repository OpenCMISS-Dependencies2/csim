/*
Copyright 2015 University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.Some license of other
*/

#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <string>

// TODO: perhaps this will move into the API one day?

namespace XmlUtils {

enum DocumentType {
    UNABLE_TO_DETERMINE_DOCUMENT_TYPE = -1,
    // TODO: do we need to differentiate version of these formats?
    DOCUMENT_TYPE_CELLML = 1,
    DOCUMENT_TYPE_SBML = 2
};

/**
 * Attempt to determine if the document located at the given URL is of a type we know about.
 * @param url The URL of the document to determine the type of.
 * @return If the URL locates a document we know about, return its document type; otherwise returns
 * XmlUtils::UNABLE_TO_DETERMINE_DOCUMENT_TYPE.
 */
enum DocumentType determineDocumentType(const std::string& url);

} // namespace XmlUtils

#endif // XML_UTILS_H

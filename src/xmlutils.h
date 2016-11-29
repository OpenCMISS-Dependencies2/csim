#ifndef XMLUTILS_H
#define XMLUTILS_H

#include <vector>

class XmlDoc
{
public:
    XmlDoc();
    ~XmlDoc();
    static std::string buildAbsoluteUri(const std::string& uri,
                                        const std::string& base);
    int parseDocument(const std::string& url);
    int parseDocumentString(const std::string& docString);
    std::string dumpString() const;
    std::string getVariableId(const std::string& xpathExpr, const std::map<std::string, std::string> &namespaces);
    std::vector<std::string> getVariableIds();
    int setXmlBase(const std::string& url);

private:
    void* mXmlDocPtr;
};

#endif // XMLUTILS_H

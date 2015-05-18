#ifndef XMLUTILS_H
#define XMLUTILS_H

class XmlDoc
{
public:
    XmlDoc();
    ~XmlDoc();
    int parseDocument(const std::string& url);
    std::string getVariableId(const std::string& xpathExpr, const std::map<std::string, std::string> &namespaces);

private:
    void* mXmlDocPtr;
};

#endif // XMLUTILS_H

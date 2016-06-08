#ifndef XMLUTILS_H
#define XMLUTILS_H

class XmlDoc
{
public:
    XmlDoc();
    ~XmlDoc();
    int parseDocument(const std::string& url);
    int parseDocumentString(const std::string& docString);
    std::string dumpString() const;
    std::string getVariableId(const std::string& xpathExpr, const std::map<std::string, std::string> &namespaces);
    int setXmlBase(const std::string& url);

private:
    void* mXmlDocPtr;
};

#endif // XMLUTILS_H

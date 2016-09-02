#include <map>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "xmlutils.h"

typedef std::map<std::string, std::string> NamespaceMap;

static xmlNodeSetPtr executeXPath(xmlDocPtr doc, const xmlChar* xpathExpr, const NamespaceMap &namespaces);

class LibXMLWrapper
{
public:
    LibXMLWrapper()
    {
        std::cout << "initialise libxml\n";
        /* Init libxml */
        xmlInitParser();
        LIBXML_TEST_VERSION
    }
    ~LibXMLWrapper()
    {
        std::cout << "terminate libxml\n";
        /* Shutdown libxml */
        xmlCleanupParser();
    }
};

static LibXMLWrapper dummyWrapper;

XmlDoc::XmlDoc() : mXmlDocPtr(0)
{
}

XmlDoc::~XmlDoc()
{
    if (mXmlDocPtr)
    {
        xmlFreeDoc(static_cast<xmlDocPtr>(mXmlDocPtr));
    }
}

int XmlDoc::parseDocument(const std::string& url)
{
    xmlDocPtr doc = xmlParseFile(url.c_str());
    if (doc == NULL)
    {
        std::cerr << "Error parsing document from URL: **" << url << "**\n";
        return -1;
    }
    mXmlDocPtr = static_cast<void*>(doc);
    return 0;
}

int XmlDoc::parseDocumentString(const std::string& ds)
{
    xmlDocPtr doc = xmlParseDoc(BAD_CAST ds.c_str());
    if (doc == NULL)
    {
        std::cerr << "Error parsing document from string." << std::endl;
        return -1;
    }
    mXmlDocPtr = static_cast<void*>(doc);
    return 0;
}

std::string XmlDoc::dumpString() const
{
    xmlDocPtr doc = static_cast<xmlDocPtr>(mXmlDocPtr);
    if (doc == NULL)
    {
        std::cerr << "Error dumping string\n";
        return "";
    }
    xmlChar *s;
    int size;
    xmlDocDumpMemory(doc, &s, &size);
    if (s == NULL)
    {
        return "";
    }
    std::string out = std::string((char*)s);
    xmlFree(s);
    return out;
}

std::string XmlDoc::getVariableId(const std::string& xpathExpr, const std::map<std::string, std::string>& namespaces)
{
    std::string id;
    xmlDocPtr doc = static_cast<xmlDocPtr>(mXmlDocPtr);
    xmlNodeSetPtr results = executeXPath(doc, BAD_CAST xpathExpr.c_str(), namespaces);
    if (results)
    {
        if (xmlXPathNodeSetGetLength(results) == 1)
        {
            xmlNodePtr node = xmlXPathNodeSetItem(results, 0);
            // we expect all XPath's to resolve into a single variable element.
            if ((node->type == XML_ELEMENT_NODE) && node->parent->name &&
                    (!strcmp((char*)(node->parent->name), "component")))
            {
                char* variableName = (char*)xmlGetProp(node, BAD_CAST "name");
                char* componentName = (char*)xmlGetProp(node->parent, BAD_CAST "name");
                id = std::string(componentName);
                id += "/";
                id += std::string(variableName);
                xmlFree(variableName);
                xmlFree(componentName);
            }
            else if ((node->type == XML_ATTRIBUTE_NODE) && node->parent->name &&
                     (!strcmp((char*)(node->parent->name), "variable")))
            {
                // for a set value change, the XPath is likely to refer to an @initial_value
                // FIXME: need to handle the case when the XPath refers to a mathml:cn with the value to set
                char* variableName = (char*)xmlGetProp(node->parent, BAD_CAST "name");
                char* componentName = (char*)xmlGetProp(node->parent->parent, BAD_CAST "name");
                id = std::string(componentName);
                id += "/";
                id += std::string(variableName);
                xmlFree(variableName);
                xmlFree(componentName);
            }
        }
        xmlXPathFreeNodeSet(results);
    }
    return id;
}

std::vector<std::string> XmlDoc::getVariableIds()
{
    std::vector<std::string> variableIds;
    xmlDocPtr doc = static_cast<xmlDocPtr>(mXmlDocPtr);
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlNodePtr currentNode = NULL;
    for (currentNode = root->children; currentNode; currentNode = currentNode->next)
    {
        if ((currentNode->type == XML_ELEMENT_NODE) &&
            (!strcmp((char*)(currentNode->name), "component")))
        {
            char* componentName = (char*)xmlGetProp(currentNode, BAD_CAST "name");
            std::string cname(componentName);
            cname += "/";
            xmlFree(componentName);
            xmlNodePtr node = NULL;
            for (node = currentNode->children; node; node = node->next)
            {
                if ((node->type == XML_ELEMENT_NODE) &&
                    (!strcmp((char*)(node->name), "variable")))
                {
                    char* variableName = (char*)xmlGetProp(node, BAD_CAST "name");
                    std::string id = cname;
                    id += std::string(variableName);
                    xmlFree(variableName);
                    variableIds.push_back(id);
                }
            }
        }
    }
    return variableIds;
}

static xmlNodeSetPtr executeXPath(xmlDocPtr doc, const xmlChar* xpathExpr, const NamespaceMap& namespaces)
{
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr results = NULL;
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL)
    {
        fprintf(stderr, "Error: unable to create new XPath context\n");
        return NULL;
    }
    for (auto it=namespaces.begin(); it!=namespaces.end(); ++it)
    {
        // As per the author of libxml2, we ignore it if the caller is trying to override the default namespace:
        //    You cannot define a default namespace for XPath, period, don't
        //    try you can't, the XPath spec does not allow it.
        if (it->first.length() > 0)
        {
            if (!(xmlXPathRegisterNs(xpathCtx, BAD_CAST it->first.c_str(), BAD_CAST it->second.c_str()) == 0))
            {
                fprintf(stderr, "Error: unable to register namespace: %s => %s\n", it->first.c_str(),
                        it->second.c_str());
                return NULL;
            }
            printf("Registering namespace: %s\n", it->first.c_str());
        }
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if (xpathObj == NULL)
    {
        fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\"\n",
                xpathExpr);
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }

    if (xmlXPathNodeSetGetLength(xpathObj->nodesetval) > 0)
    {
        int i;
        results = xmlXPathNodeSetCreate(NULL);
        for (i=0; i< xmlXPathNodeSetGetLength(xpathObj->nodesetval); ++i)
            xmlXPathNodeSetAdd(results, xmlXPathNodeSetItem(xpathObj->nodesetval, i));
    }
    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    return results;
}

int XmlDoc::setXmlBase(const std::string& url)
{
    xmlDocPtr doc = static_cast<xmlDocPtr>(mXmlDocPtr);
    xmlNodePtr rootElement = xmlDocGetRootElement(doc);
    xmlNodeSetBase(rootElement, BAD_CAST (url.c_str()));
    return 0;
}

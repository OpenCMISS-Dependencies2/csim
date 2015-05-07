
#include "cellml_model_definition.h"

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include <IfaceCellML_APISPEC.hxx>
#include <IfaceCCGS.hxx>
#include <CeVASBootstrap.hpp>
#include <MaLaESBootstrap.hpp>
#include <CCGSBootstrap.hpp>
#include <CellMLBootstrap.hpp>
#include <IfaceAnnoTools.hxx>
#include <AnnoToolsBootstrap.hpp>
#include <cellml-api-cxx-support.hpp>

/*
 * Prototype local methods
 */
static std::wstring s2ws(const std::string& str);
//static std::string ws2s(const std::wstring& wstr);


CellmlModelDefinition::CellmlModelDefinition() : mUrl("")
{
}

CellmlModelDefinition::~CellmlModelDefinition()
{
    // nothing to do yet?
}

int CellmlModelDefinition::loadModel(const std::string &url)
{
    std::cout << "Creating CellML Model Definition from the URL: "
              << url << std::endl;
    mUrl = url;
    if (mUrl.empty()) return -1;
    std::wstring urlW = s2ws(url);
    ObjRef<iface::cellml_api::CellMLBootstrap> cb = CreateCellMLBootstrap();
    ObjRef<iface::cellml_api::DOMModelLoader> ml = cb->modelLoader();
    ObjRef<iface::cellml_api::Model> model;
    try
    {
        model = ml->loadFromURL(urlW);
        model->fullyInstantiateImports();
        mModel = static_cast<void*>(model);
    }
    catch (...)
    {
      std::wcerr << L"Error loading model: " << urlW << std::endl;
      mModel = static_cast<void*>(NULL);
    }
    return 0;
}

std::wstring s2ws(const std::string& str)
{
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

/*std::string ws2s(const std::wstring& wstr)
{
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}*/

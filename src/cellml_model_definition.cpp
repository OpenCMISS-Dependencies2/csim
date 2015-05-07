
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
static std::string ws2s(const std::wstring& wstr);

#define StateType 1
#define KnownType 2
#define WantedType 3
#define IndependentType 4

CellmlModelDefinition::CellmlModelDefinition() : mUrl(""), mModelLoaded(false), mModel(0),
    mAnnotations(0), mCevas(0), mCodeInformation(0)
{
    mNumberOfIndependentVariables = 0;
    mNumberOfKnownVariables = 0;
    mNumberOfWantedVariables = 0;
    mStateCounter = 0;
}

CellmlModelDefinition::~CellmlModelDefinition()
{
    if (mModel)
    {
        iface::cellml_api::Model* el = static_cast<iface::cellml_api::Model*>(mModel);
        el->release_ref();
    }
    if (mAnnotations)
    {
        iface::cellml_services::AnnotationSet* el =
                static_cast<iface::cellml_services::AnnotationSet*>(mAnnotations);
        el->release_ref();
    }
    if (mCevas)
    {
        iface::cellml_services::CeVAS* el =
                static_cast<iface::cellml_services::CeVAS*>(mCevas);
        el->release_ref();
    }
    if (mCodeInformation)
    {
        iface::cellml_services::CodeInformation* el =
                static_cast<iface::cellml_services::CodeInformation*>(mCodeInformation);
        el->release_ref();
    }
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
    try
    {
        ObjRef<iface::cellml_api::Model> model = ml->loadFromURL(urlW);
        model->fullyInstantiateImports();
        model->add_ref();
        mModel = static_cast<void*>(model);
        // create an annotation set to manage our variable usages
        ObjRef<iface::cellml_services::AnnotationToolService> ats = CreateAnnotationToolService();
        ObjRef<iface::cellml_services::AnnotationSet> as = ats->createAnnotationSet();
        as->add_ref();
        mAnnotations = static_cast<void*>(as);
        // mapping the connections between variables is a very expensive operation, so we want to
        // only do it once and keep hold of the mapping (tracker item 3294)
        ObjRef<iface::cellml_services::CeVASBootstrap> cvbs = CreateCeVASBootstrap();
        ObjRef<iface::cellml_services::CeVAS> cevas = cvbs->createCeVASForModel(model);
        std::wstring msg = cevas->modelError();
        if (msg != L"")
        {
            std::cerr << "loadModel: Error creating CellML Variable Association Service: "
                      << ws2s(msg) << std::endl;
            return -2;
        }
        cevas->add_ref();
        mCevas = static_cast<void*>(cevas);
        // now check we can generate code and grab hold of the initial code information
        ObjRef<iface::cellml_services::CodeGeneratorBootstrap> cgb = CreateCodeGeneratorBootstrap();
        ObjRef<iface::cellml_services::CodeGenerator> cg = cgb->createCodeGenerator();
        try
        {
            cg->useCeVAS(cevas);
            ObjRef<iface::cellml_services::CodeInformation> cci = cg->generateCode(model);
            msg = cci->errorMessage();
            if (msg != L"")
            {
                std::cerr << "CellmlModelDefintion::loadModel: Error generating code: "
                          << ws2s(msg) << std::endl;
                return -4;
            }
            // TODO: we are only interested in models we can work with?
            if (cci->constraintLevel() != iface::cellml_services::CORRECTLY_CONSTRAINED)
            {
                std::cerr << "CellmlModelDefintion::loadModel: Model is not correctly constrained: "
                          << std::endl;
                return -5;
            }
            cci->add_ref();
            mCodeInformation = static_cast<void*>(cci);
            // and add all state variables as wanted and the variable of integration as known
            ObjRef<iface::cellml_services::ComputationTargetIterator> cti = cci->iterateTargets();
            while (true)
            {
                ObjRef<iface::cellml_services::ComputationTarget> ct = cti->nextComputationTarget();
                if (ct == NULL) break;
                ObjRef<iface::cellml_api::CellMLVariable> v(ct->variable());
                if (ct->type() == iface::cellml_services::STATE_VARIABLE)
                {
                    try
                    {
                        if (v->initialValue() != L"") mInitialValues.insert(
                                    std::pair<std::pair<int,int>, double>(
                                        std::pair<int,int>(StateType, mStateCounter),
                                        v->initialValueValue()
                                        ));
                    } catch (...) {}
                    mVariableTypes[v->objid()] = StateType;
                    mVariableIndices[v->objid()] = mStateCounter;
                    mStateCounter++;
                }
                else if (ct->type() == iface::cellml_services::VARIABLE_OF_INTEGRATION)
                {
                    mVariableTypes[v->objid()] = IndependentType;
                    mNumberOfIndependentVariables++;
                }
            }
        }
        catch (...)
        {
            std::cerr << "loadModel: Error generating the code information for the model" << std::endl;
            return -3;
        }

        // if we get to here, everything worked.
        mModelLoaded = true;
    }
    catch (...)
    {
      std::wcerr << L"Error loading model: " << urlW << std::endl;
      mModel = static_cast<void*>(NULL);
      return -1;
    }
    return 0;
}

std::wstring s2ws(const std::string& str)
{
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

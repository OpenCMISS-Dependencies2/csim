
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

#include "csim/error_codes.h"

/*
 * Prototype local methods
 */
static std::wstring s2ws(const std::string& str);
static std::string ws2s(const std::wstring& wstr);

#define StateType 1
#define InputType 2
#define OutputType 3
#define IndependentType 4

class CellmlApiObjects
{
public:
    ObjRef<iface::cellml_api::Model> model;
    ObjRef<iface::cellml_services::AnnotationSet> annotations;
    ObjRef<iface::cellml_services::CeVAS> cevas;
    ObjRef<iface::cellml_services::CodeInformation> codeInformation;
};

CellmlModelDefinition::CellmlModelDefinition() : mUrl(""), mModelLoaded(false), mCapi(0)
{
    mNumberOfIndependentVariables = 0;
    mNumberOfInputVariables = 0;
    mNumberOfOutputVariables = 0;
    mStateCounter = 0;
}

CellmlModelDefinition::~CellmlModelDefinition()
{
    if (mCapi)
    {
        delete mCapi;
        mCapi = NULL;
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
        // we have a model, so we can start grabbing hold of the CellML API objects
        mCapi = new CellmlApiObjects();
        mCapi->model = model;
        // create an annotation set to manage our variable usages
        ObjRef<iface::cellml_services::AnnotationToolService> ats = CreateAnnotationToolService();
        ObjRef<iface::cellml_services::AnnotationSet> as = ats->createAnnotationSet();
        mCapi->annotations = as;
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
        mCapi->cevas = cevas;
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
            mCapi->codeInformation = cci;
            // and add all state variables as output and the variable of integration as input
            ObjRef<iface::cellml_services::ComputationTargetIterator> cti = cci->iterateTargets();
            while (true)
            {
                ObjRef<iface::cellml_services::ComputationTarget> ct = cti->nextComputationTarget();
                if (ct == NULL) break;
                ObjRef<iface::cellml_api::CellMLVariable> v(ct->variable());
                if (ct->type() == iface::cellml_services::STATE_VARIABLE)
                {
                    /*try
                    {
                        if (v->initialValue() != L"") mInitialValues.insert(
                                    std::pair<std::pair<int,int>, double>(
                                        std::pair<int,int>(StateType, mStateCounter),
                                        v->initialValueValue()
                                        ));
                    } catch (...) {}*/
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
      return -1;
    }
    return csim::CSIM_OK;
}

int CellmlModelDefinition::setVariableAsInput(const std::string &variableId)
{
    std::cout << "CellmlModelDefinition::setVariableAsInput: flagging variable: " << variableId
              << "; as a INPUT variable." << std::endl;
    std::vector<iface::cellml_services::VariableEvaluationType> vets;
    // initially, only allow "constant" variables to be defined externally
    vets.push_back(iface::cellml_services::CONSTANT);
    //vets.push_back(iface::cellml_services::VARIABLE_OF_INTEGRATION);
    //vets.push_back(iface::cellml_services::FLOATING);
    return csim::UNABLE_TO_FLAG_VARIABLE_INPUT;
}

int CellmlModelDefinition::setVariableAsOutput(const std::string &variableId)
{
    std::cout << "CellmlModelDefinition::setVariableAsOutput: flagging variable: " << variableId
              << "; as a OUTPUT variable." << std::endl;
    std::vector<iface::cellml_services::VariableEvaluationType> vets;
    // state variables should be flagged if they need to be copied into the wanted array
    vets.push_back(iface::cellml_services::STATE_VARIABLE);
    vets.push_back(iface::cellml_services::PSEUDOSTATE_VARIABLE);
    vets.push_back(iface::cellml_services::ALGEBRAIC);
    //vets.push_back(iface::cellml_services::LOCALLY_BOUND);
    // we need to allow constant variables to be flagged as wanted since if it is a model with no
    // differential equations then all algebraic variables will be constant - i.e., constitutive laws
    vets.push_back(iface::cellml_services::CONSTANT);
    return csim::UNABLE_TO_FLAG_VARIABLE_OUTPUT;
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

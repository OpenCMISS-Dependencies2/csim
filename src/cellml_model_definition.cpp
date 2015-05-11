
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
static int flagVariable(const std::string& variableId, unsigned char type,
                        std::vector<iface::cellml_services::VariableEvaluationType> vets,
                        int& count, CellmlApiObjects* capi,
                        std::map<std::string, unsigned char>& variableTypes,
                        std::map<std::string, std::map<unsigned char, int> >& mVariableIndices);
static ObjRef<iface::cellml_api::CellMLVariable> findLocalVariable(CellmlApiObjects* capi, const std::string& variableId);
typedef std::pair<std::string, std::string> CVpair;
static CVpair splitName(const std::string& s);

/**
 * In order to be flexible in allowing a single variable to be multiple types (state
 * and output; input and output; etc.) we use these bit flags.
 * http://www.cplusplus.com/forum/general/1590/
 */
enum VariableTypes {
    StateType        = 0x01,
    InputType        = 0x02,
    OutputType       = 0x04,
    IndependentType  = 0x08,
  //OpFullscreen    = 0x10,
  //OpDaylight      = 0x20
};

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
            // flag all state variables and the variable of integration
            ObjRef<iface::cellml_services::ComputationTargetIterator> cti = cci->iterateTargets();
            while (true)
            {
                ObjRef<iface::cellml_services::ComputationTarget> ct = cti->nextComputationTarget();
                if (ct == NULL) break;
                ObjRef<iface::cellml_api::CellMLVariable> v(ct->variable());
                if (ct->type() == iface::cellml_services::STATE_VARIABLE)
                {
                    mVariableTypes[v->objid()] = StateType;
                    mVariableIndices[v->objid()][StateType] = mStateCounter;
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
    int index = flagVariable(variableId, InputType, vets,
                             mNumberOfInputVariables, mCapi,
                             mVariableTypes, mVariableIndices);
    return index;
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
    int index = flagVariable(variableId, OutputType, vets,
                             mNumberOfOutputVariables, mCapi, mVariableTypes,
                             mVariableIndices);
    return index;
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

int flagVariable(const std::string& variableId, unsigned char type,
                 std::vector<iface::cellml_services::VariableEvaluationType> vets,
                 int& count, CellmlApiObjects* capi,
                 std::map<std::string, unsigned char>& variableTypes,
                 std::map<std::string, std::map<unsigned char, int> >& variableIndices)
{
    if (! capi->codeInformation)
    {
        std::cerr << "CellML Model Definition::flagVariable: missing model implementation?" << std::endl;
        return csim::UNABLE_TO_FLAG_VARIABLE;
    }
    ObjRef<iface::cellml_api::CellMLVariable> sv = findLocalVariable(capi, variableId);
    if (!sv)
    {
        std::cerr << "CellML Model Definition::flagVariable: unable to find source variable for: "
                  << variableId << std::endl;
        return csim::UNABLE_TO_FLAG_VARIABLE;
    }

    // FIXME: here we simply accept any flag combination. Code generation is the place
    // where flags will be checked for consistency?

    // check if source is already flagged with the specified type.
    std::map<std::string, unsigned char>::iterator currentAnnotation =
            variableTypes.find(sv->objid());
    unsigned char currentTypes;
    if (currentAnnotation != variableTypes.end())
    {
        currentTypes = currentAnnotation->second;
        if (currentTypes & type)
        {
            std::cout << "Already flagged same type, nothing to do." << std::endl;
            return variableIndices[sv->objid()][type];
        }
    }
    // find corresponding computation target
    ObjRef<iface::cellml_services::ComputationTargetIterator> cti =
            capi->codeInformation->iterateTargets();
    ObjRef<iface::cellml_services::ComputationTarget> ct(NULL);
    while(true)
    {
        ct = cti->nextComputationTarget();
        if (ct == NULL) break;
        if (ct->variable() == sv) break;
    }
    if (!ct)
    {
        std::cerr << "CellMLModelDefinition::flagVariable -- unable get computation target for the source of variable: "
                  << variableId << std::endl;
        return csim::NO_MATCHING_COMPUTATION_TARGET;
    }

    // check type of computation target and make sure compatible
    unsigned int i,compatible = 0;
    for (i=0; i<vets.size(); i++)
    {
        if (ct->type() == vets[i])
        {
            compatible = 1;
            break;
        }
    }
    if (!compatible)
    {
        std::cerr << "CellMLModelDefinition::flagVariable -- computation target for variable: "
                  << variableId << "; is the wrong type to be flagged" << std::endl;
        std::cerr << "Computation target for this source variable is: ";
        switch (ct->type())
        {
        case iface::cellml_services::CONSTANT:
            std::cerr << "CONSTANT";
            break;
        case iface::cellml_services::VARIABLE_OF_INTEGRATION:
            std::cerr << "VARIABLE_OF_INTEGRATION";
            break;
        case iface::cellml_services::STATE_VARIABLE:
            std::cerr << "STATE_VARIABLE";
            break;
        case iface::cellml_services::PSEUDOSTATE_VARIABLE:
            std::cerr << "PSEUDOSTATE_VARIABLE";
            break;
        case iface::cellml_services::ALGEBRAIC:
            std::cerr << "ALGEBRAIC";
            break;
        case iface::cellml_services::LOCALLY_BOUND:
            std::cerr << "LOCALLY_BOUND";
            break;
        case iface::cellml_services::FLOATING:
            std::cerr << "FLOATING";
            break;
        default:
            std::cerr << "Invalid";
        }
        std::cerr << std::endl;
        ct->release_ref();
        return csim::MISMATCHED_COMPUTATION_TARGET;
    }
    variableTypes[sv->objid()] = currentTypes | type;
    variableIndices[sv->objid()][type] = count++;
    return variableIndices[sv->objid()][type];
}

ObjRef<iface::cellml_api::CellMLVariable> findLocalVariable(CellmlApiObjects* capi, const std::string& variableId)
{
    if (!(capi->cevas))
    {
        std::cerr << "CellMLModelDefinition::findLocalVariable -- missing CeVAS object?" << std::endl;
        return NULL;
    }
    // find named variable - in local components only!
    CVpair cv = splitName(variableId);
    if ((cv.first.length() == 0) || (cv.second.length() == 0)) return NULL;
    //std::cout << "Component name: " << cv.first << "; variable name: " << cv.second << std::endl;
    ObjRef<iface::cellml_api::CellMLComponentSet> components = capi->model->localComponents();
    ObjRef<iface::cellml_api::CellMLComponent> component = components->getComponent(s2ws(cv.first));
    if (!component)
    {
        std::cerr << "CellMLModelDefinition::findLocalVariable -- unable to find local component: " << cv.first << std::endl;
        return NULL;
    }
    ObjRef<iface::cellml_api::CellMLVariableSet> variables = component->variables();
    ObjRef<iface::cellml_api::CellMLVariable> variable = variables->getVariable(s2ws(cv.second));
    if (!variable)
    {
        std::cerr << "CellMLModelDefinition::findLocalVariable -- unable to find variable: " << cv.first << " / "
                  << cv.second << std::endl;
        return NULL;
    }
    // get source variable
    ObjRef<iface::cellml_services::ConnectedVariableSet> cvs = capi->cevas->findVariableSet(variable);
    ObjRef<iface::cellml_api::CellMLVariable> v = cvs->sourceVariable();
    if (!v)
    {
        std::cerr << "CellMLModelDefinition::findLocalVariable -- unable get source variable for variable: "
                  << cv.first << " / " << cv.second << std::endl;
        return NULL;
    }
    return v;
}

CVpair splitName(const std::string& s)
{
    CVpair p;
    std::size_t pos = s.find('/');
    if (pos == std::string::npos) return p;
    p.first = s.substr(0, pos);
    p.second = s.substr(pos+1);
    return p;
}


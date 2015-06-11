#include <iostream>
#include <string>
#include <vector>
#include <math.h>

#include <cvode/cvode.h>             /* main integrator header file */
#include <cvode/cvode_dense.h>       /* use CVDENSE linear solver */
#include <cvode/cvode_band.h>        /* use CVBAND linear solver */
#include <cvode/cvode_diag.h>        /* use CVDIAG linear solver */
#include <nvector/nvector_serial.h>  /* serial N_Vector types, fct. and macros */
#include <sundials/sundials_types.h> /* definition of realtype */
#include <sundials/sundials_math.h>  /* contains the macros ABS, SUNSQR, and EXP*/

#include <csim/model.h>
#include <csim/error_codes.h>
#include <csim/executable_functions.h>

/* Shared Problem Constants */

#define ATOL RCONST(1.0e-6)
#define RTOL RCONST(0.0)

/* Functions called by CVODE */
static int f(realtype t, N_Vector y, N_Vector ydot, void *user_data);

/* Private function to check function return values */
static int check_flag(void *flagvalue, const char *funcname, int opt);

// My user data class
struct UserData
{
    csim::ModelFunction modelFunction;
    N_Vector states, inputs, outputs;
};

void usage(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "CSim example: CVODE integrator\n"
                  << argv[0] << " <CellML model file> <output variable 1> [output 2...]" << std::endl;
        std::cerr << "\tOutput variables should be indentified using component_name/variable_name\n"
                  << "\tAt least one output must be specified. " << std::endl;
        exit(-1);
    }
}

void printResults(UserData& ud)
{
    for (int i = 0; i < NV_LENGTH_S(ud.outputs); ++i)
    {
        if (i != 0) std::cout << "\t";
        std::cout << NV_Ith_S(ud.outputs, i);
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    // for collecting data passed into CVODE
    UserData ud;
    // define the simulation for now
    double x0 = 0, x1 = M_PI * 2.0;
    int nSteps = 100;
    // check command line arguments
    usage(argc, argv);
    csim::Model model;
    model.loadCellmlModel(argv[1]);
    // output variables
    std::vector<std::string> outputNames;
    std::vector<int> outputIndicies;
    int error = 0;
    for (int i = 2; i < argc; ++i)
    {
        outputNames.push_back(argv[i]);
        int index = model.setVariableAsOutput(argv[i]);
        if (index < 0) error++;
        else outputIndicies.push_back(index);
    }
    if (error)
    {
        std::cerr << "There was an error setting output variables." << std::endl;
        return -2;
    }
    if (model.instantiate() != csim::CSIM_OK)
    {
        std::cerr << "There was an error instantiating the model." << std::endl;
        return -3;
    }
    // grab the model's executable functions
    csim::InitialiseFunction initFunction = model.getInitialiseFunction();
    ud.modelFunction = model.getModelFunction();
    if ((initFunction == NULL) || (ud.modelFunction == NULL))
    {
        std::cerr << "Unable to get the model's executable function(s)." << std::endl;
        return -4;
    }
    ud.states = N_VNew_Serial(model.numberOfStateVariables());
	N_Vector rates = N_VNew_Serial(model.numberOfStateVariables());
    ud.inputs = N_VNew_Serial(0);
    ud.outputs = N_VNew_Serial(outputNames.size());
    // now get CVODE set up
    double reltol = RTOL, abstol = ATOL;
    // initialise the inputs and initial values of the state variables
    initFunction(NV_DATA_S(ud.states), NV_DATA_S(ud.outputs), NV_DATA_S(ud.inputs));
    // and calculate and print the initial state of the model
    ud.modelFunction(x0, NV_DATA_S(ud.states), NV_DATA_S(rates), NV_DATA_S(ud.outputs), NV_DATA_S(ud.inputs));
    std::cout << "results headed goes here" << std::endl;
    printResults(ud);

    // create and initialise our CVODE integrator
    void* cvode_mem = CVodeCreate(CV_ADAMS, CV_FUNCTIONAL);
    if(check_flag((void *)cvode_mem, "CVodeCreate", 0)) return(1);
    int flag = CVodeInit(cvode_mem, f, x0, ud.states);
    if(check_flag(&flag, "CVodeInit", 1)) return(1);
    flag = CVodeSStolerances(cvode_mem, reltol, abstol);
    if(check_flag(&flag, "CVodeSStolerances", 1)) return(1);

    // add our user data
    flag = CVodeSetUserData(cvode_mem, (void*)(&ud));
    if (check_flag(&flag,"CVodeSetUserData",1)) return(1);

    double dx = (x1 - x0) / nSteps;
    double x = x0, xout = x0;
    for (int i = 0; i < nSteps; ++i)
    {
        xout += dx;
        flag = CVodeSetStopTime(cvode_mem, xout);
        if (check_flag(&flag,"CVodeSetStopStime",1)) return(1);
        flag = CVode(cvode_mem, xout, ud.states, &x, CV_NORMAL);
        if (check_flag(&flag,"CVode",1)) return(1);
        // call the model's function to make sure all the outputs are at the current time
        ud.modelFunction(xout, NV_DATA_S(ud.states), NV_DATA_S(rates), NV_DATA_S(ud.outputs), NV_DATA_S(ud.inputs));
        printResults(ud);
    }
    return 0;
}

int f(realtype x, N_Vector y, N_Vector ydot, void *user_data)
{
    UserData* ud = (UserData*)user_data;
    ud->modelFunction(x, NV_DATA_S(y), NV_DATA_S(ydot), NV_DATA_S(ud->outputs), NV_DATA_S(ud->inputs));
    return 0;
}

static int check_flag(void *flagvalue, const char *funcname, int opt)
{
  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return(1); }

  /* Check if flag < 0 */
  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
              funcname, *errflag);
      return(1); }}

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL) {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return(1); }

  return(0);
}

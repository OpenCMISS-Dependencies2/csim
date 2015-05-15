#include "gtest/gtest.h"

#include "csim/model.h"
#include "csim/executable_functions.h"
#include "csim/error_codes.h"

// generated with test resource locations
#include "test_resources.h"

TEST(Execution, function_retrieval) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE)));
    EXPECT_EQ(0, model.setVariableAsInput("main/deriv_approx_initial_value"));
    EXPECT_EQ(0, model.setVariableAsOutput("main/sin1"));
    EXPECT_EQ(1, model.setVariableAsOutput("main/sin2"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(csim::CSIM_OK, model.instantiate());
    csim::InitialiseFunction initFunction = model.getInitialiseFunction();
    EXPECT_TRUE(initFunction != NULL);
    csim::ModelFunction modelFunction = model.getModelFunction();
    EXPECT_TRUE(modelFunction != NULL);
}

TEST(Execution, function_execution) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_MODEL_RESOURCE)));
    EXPECT_EQ(0, model.setVariableAsOutput("actual_sin/sin"));
    EXPECT_EQ(1, model.setVariableAsOutput("deriv_approx_sin/sin"));
    EXPECT_EQ(0, model.setVariableAsInput("main/deriv_approx_initial_value"));
    // check that source variables are correctly resolved
    EXPECT_EQ(0, model.setVariableAsOutput("main/sin1"));
    EXPECT_EQ(1, model.setVariableAsOutput("main/sin2"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(csim::CSIM_OK, model.instantiate(true));
    csim::InitialiseFunction initFunction = model.getInitialiseFunction();
    EXPECT_TRUE(initFunction != NULL);
    csim::ModelFunction modelFunction = model.getModelFunction();
    EXPECT_TRUE(modelFunction != NULL);
    double states[10], rates[10], inputs[1], outputs[3], x;
    // make sure the values get set correctly on initialisation
    inputs[0] = -123;
    states[0] = -321;
    initFunction(states, inputs);
    EXPECT_EQ(0.0, states[0]);
    EXPECT_EQ(0.0, inputs[0]);
    outputs[0] = 123;
    outputs[1] = 456;
    outputs[2] = 789;
    x = 0.0;
    modelFunction(x, states, rates, outputs, inputs);
    EXPECT_EQ(0.0, outputs[0]);
    EXPECT_EQ(0.0, outputs[1]);
    EXPECT_EQ(0.0, outputs[2]);
    // and that we can override default inital values
    states[0] = 1.0;
    modelFunction(x, states, rates, outputs, inputs);
    EXPECT_EQ(1.0, outputs[1]);
}

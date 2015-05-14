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

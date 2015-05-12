#include "gtest/gtest.h"

#include "csim/model.h"
#include "csim/error_codes.h"

// generated with test resource locations
#include "test_resources.h"

TEST(Model, new_model) {
    csim::Model model;
    EXPECT_FALSE(model.isModelInstantiated());
}

TEST(Model, load_nonexisting_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel("http://example.com/this.cellml.file.should.never.exist.xml"));
}

TEST(Model, load_non_cellml_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel(TestResources::getLocation(TestResources::SBML_MODEL_RESOURCE)));
}

TEST(Model, load_invalid_cellml_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_INVALID_MODEL_RESOURCE)));
}

TEST(Model, load_valid_cellml_model) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_MODEL_RESOURCE)));
}

TEST(Model, load_valid_cellml_model_with_imports) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE)));
}

TEST(Model, set_valid_io_variables) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE)));
    EXPECT_EQ(0, model.setVariableAsInput("main/deriv_approx_initial_value"));
    EXPECT_EQ(0, model.setVariableAsOutput("main/sin1"));
    EXPECT_EQ(1, model.setVariableAsOutput("main/sin2"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(3, model.setVariableAsOutput("main/deriv_approx_initial_value"));
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_MODEL_RESOURCE)));
    EXPECT_EQ(0, model.setVariableAsOutput("actual_sin/sin"));
    EXPECT_EQ(1, model.setVariableAsOutput("deriv_approx_sin/sin"));
    // check that source variables are correctly resolved
    EXPECT_EQ(0, model.setVariableAsOutput("main/sin1"));
    EXPECT_EQ(1, model.setVariableAsOutput("main/sin2"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
}

TEST(Model, set_invalid_io_variables) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_MODEL_RESOURCE)));
    EXPECT_EQ(csim::UNABLE_TO_FLAG_VARIABLE, model.setVariableAsInput("main/z"));
    EXPECT_EQ(csim::UNABLE_TO_FLAG_VARIABLE, model.setVariableAsOutput("main/xx"));
    EXPECT_EQ(0, model.setVariableAsOutput("deriv_approx_sin/sin"));
    EXPECT_EQ(csim::UNABLE_TO_FLAG_VARIABLE, model.setVariableAsOutput("main/xx"));
    EXPECT_EQ(csim::MISMATCHED_COMPUTATION_TARGET, model.setVariableAsInput("deriv_approx_sin/sin"));
}


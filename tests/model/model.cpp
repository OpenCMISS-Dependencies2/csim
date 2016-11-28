#include "gtest/gtest.h"

#include "csim/model.h"
#include "csim/error_codes.h"

// generated with test resource locations
#include "test_resources.h"

TEST(Model, new_model) {
    csim::Model model;
    EXPECT_FALSE(model.isInstantiated());
}

TEST(Model, load_nonexisting_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel("http://example.com/this.cellml.file.should.never.exist.xml"));
}

// since https is always messing up the CellML API better test it works.
TEST(Model, load_remote_model) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel("https://models.physiomeproject.org/w/andre/sine/rawfile/46ffb5e4c20e6b4243be2c5b71f5c89744092645/sin_approximations_import.xml"));
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

TEST(Model, map_xpath_to_variable_id) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE)));
    std::map<std::string, std::string> ns;
    ns["cellml"] = "http://www.cellml.org/cellml/1.1#";
    EXPECT_EQ("main/sin1", model.mapXpathToVariableId(
                  "/cellml:model/cellml:component[@name='main']/cellml:variable[@name='sin1']",
                  ns));
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_MODEL_RESOURCE)));
    ns["cellml"] = "http://www.cellml.org/cellml/1.0#";
    EXPECT_EQ("actual_sin/x", model.mapXpathToVariableId(
                  "//cellml:component[@name='actual_sin']/cellml:variable[@name='x']",
                  ns));
    EXPECT_EQ("main/x", model.mapXpathToVariableId(
                  "//cellml:component[@name='main']/cellml:variable[@name='x']/@initial_value",
                  ns));
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

TEST(Model, instantiation) {
    csim::Model model;
    EXPECT_EQ(csim::CSIM_OK,
              model.loadCellmlModel(TestResources::getLocation(TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE)));
    EXPECT_EQ(0, model.setVariableAsInput("main/deriv_approx_initial_value"));
    EXPECT_EQ(0, model.setVariableAsOutput("main/sin1"));
    EXPECT_EQ(1, model.setVariableAsOutput("main/sin2"));
    EXPECT_EQ(2, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(csim::CSIM_OK, model.instantiate());
    // check that variable flagging fails after model instantiated.
    EXPECT_EQ(csim::MODEL_ALREADY_INSTANTIATED, model.setVariableAsOutput("main/sin3"));
    EXPECT_EQ(csim::MODEL_ALREADY_INSTANTIATED, model.setVariableAsInput("main/sin3"));
}

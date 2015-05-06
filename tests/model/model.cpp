#include "gtest/gtest.h"

#include "csim/model.h"
#include "csim/error_codes.h"

TEST(Model, new_model) {
    csim::Model model;
    EXPECT_FALSE(model.isModelInstantiated());
}

TEST(Model, load_nonexisting_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel("http://example.com/this.cellml.file.should.never.exist.xml"));
}

TEST(Model, load_noncellml_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL,
              model.loadCellmlModel("sbml-model.xml"));
}


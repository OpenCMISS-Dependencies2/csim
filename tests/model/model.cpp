#include "gtest/gtest.h"

#include "csim/model.h"
#include "csim/error_codes.h"

TEST(Model, new_model) {
    csim::Model model;
    EXPECT_FALSE(model.isModelInstantiated());
}

TEST(Model, load_nonexisting_model) {
    csim::Model model;
    EXPECT_EQ(csim::UNABLE_TO_LOAD_MODEL_URL, model.loadModel("http://example.com/this.cellml.file.should.never.exist.xml"));
}


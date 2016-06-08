#include "gtest/gtest.h"

#include <string>

#include "csimsbw.h"
#include "csim/error_codes.h"

// generated with test resource locations
#include "test_resources.h"

TEST(SBW, say_hello) {
    char* hello;
    int length;
    int code = csim_sayHello(&hello, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 11);
    EXPECT_EQ(std::string(hello), "Hello World");
    csim_freeVector(hello);
}

TEST(SBW, model_string) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    EXPECT_EQ(code, 0);
    int expectedLength = 9422 + strlen(" xml:base=\"\"") +
            strlen(TestResources::getLocation(
                       TestResources::CELLML_SINE_MODEL_RESOURCE));
    EXPECT_EQ(length, expectedLength);
    EXPECT_NE(std::string(modelString), "");
    csim_freeVector(modelString);
}

TEST(SBW, model_with_imports_string) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    EXPECT_EQ(code, 0);
    int expectedLength = 4629 + strlen(" xml:base=\"\"") +
            strlen(TestResources::getLocation(
                       TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE));
    EXPECT_EQ(length, expectedLength);
    EXPECT_NE(std::string(modelString), "");
    csim_freeVector(modelString);
}

TEST(SBW, load_model) {
    char* modelString;
    int length;
    // need to allow users to apply changes to the raw XML (from SED-ML for example)
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    EXPECT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    EXPECT_EQ(code, 0);
    csim_freeVector(modelString);
}

TEST(SBW, load_model_with_imports) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    EXPECT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    EXPECT_EQ(code, 0);
    csim_freeVector(modelString);
}

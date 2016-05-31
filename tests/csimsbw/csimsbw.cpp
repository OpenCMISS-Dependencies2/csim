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
    free(hello);
}

TEST(SBW, model_string) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 4629);
    EXPECT_NE(std::string(modelString), "");
    free(modelString);
}


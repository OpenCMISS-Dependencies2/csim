#include "gtest/gtest.h"

#include <string>
#include <cmath>

#include "csimsbw.h"
#include "csim/error_codes.h"

// generated with test resource locations
#include "test_resources.h"

#define ABS_TOL 1.0e-7
#define LOOSE_TOL 1.0e-1

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

TEST(SBW, get_variables) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    char** variables;
    code = csim_getVariables(&variables, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 19);
    // test a few random variable IDs
    EXPECT_EQ(std::string(variables[0]), "actual_sin/sin");
    EXPECT_EQ(std::string(variables[2]), "deriv_approx_sin/sin");
    EXPECT_EQ(std::string(variables[5]), "main/deriv_approx_initial_value");
    EXPECT_EQ(std::string(variables[9]), "main/x");
    EXPECT_EQ(std::string(variables[15]), "parabolic_approx_sin/kPi_32");
    EXPECT_EQ(std::string(variables[10]), "parabolic_approx_sin/C");
    csim_freeMatrix((void**)variables, length);
}

TEST(SBW, get_variables_with_imports) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    char** variables;
    code = csim_getVariables(&variables, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 5);
    // test a few random variable IDs
    EXPECT_EQ(std::string(variables[1]), "main/sin1");
    EXPECT_EQ(std::string(variables[3]), "main/sin3");
    EXPECT_EQ(std::string(variables[4]), "main/x");
    csim_freeMatrix((void**)variables, length);
}

TEST(SBW, get_initial_values) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    code = csim_getValues(&values, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 19);
    // test a few random variable values
    EXPECT_NEAR(values[0], 0.0, ABS_TOL);
    EXPECT_NEAR(values[5], 0.0, ABS_TOL);
    EXPECT_NEAR(values[10], 0.75, ABS_TOL);
    csim_freeVector(values);
}

TEST(SBW, get_initial_values_import) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    code = csim_getValues(&values, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 5);
    // test a few random variable values
    EXPECT_NEAR(values[1], 0.0, ABS_TOL);
    EXPECT_NEAR(values[3], 0.0, ABS_TOL);
    EXPECT_NEAR(values[4], 0.0, ABS_TOL);
    csim_freeVector(values);
}

TEST(SBW, set_value) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    // get the initial values
    code = csim_getValues(&values, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 19);
    // test a few random variable values
    EXPECT_NEAR(values[0], 0.0, ABS_TOL);
    EXPECT_NEAR(values[5], 0.0, ABS_TOL);
    EXPECT_NEAR(values[10], 0.75, ABS_TOL);
    csim_freeVector(values);
    // and now we should be able to set the value of the input variables
    code = csim_setValue("main/deriv_approx_initial_value", 123.456);
    EXPECT_EQ(code, 0);
    code = csim_setValue("parabolic_approx_sin/C", 987.654);
    EXPECT_EQ(code, 0);
    code = csim_getValues(&values, &length);
    EXPECT_NEAR(values[5], 123.456, ABS_TOL);
    EXPECT_NEAR(values[10], 987.654, ABS_TOL);
    csim_freeVector(values);
}

TEST(SBW, set_value_import) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    code = csim_getValues(&values, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 5);
    // test a few random variable values
    EXPECT_NEAR(values[1], 0.0, ABS_TOL);
    EXPECT_NEAR(values[3], 0.0, ABS_TOL);
    EXPECT_NEAR(values[4], 0.0, ABS_TOL);
    csim_freeVector(values);
    // try setting a variable (there are no inputs in this model)
    code = csim_setValue("main/sin1", 123.4);
    EXPECT_NE(code, 0);
}

TEST(SBW, one_step) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    // get the initial values
    code = csim_getValues(&values, &length);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(length, 19);
    csim_freeVector(values);
    code = csim_setTolerances(1.0, 1.0, 10);
    code = csim_oneStep(1.5);
    EXPECT_EQ(code, 0);
    // check the outputs
    code = csim_getValues(&values, &length);
    EXPECT_NEAR(values[9], 1.5, ABS_TOL); // main/x
    EXPECT_NEAR(values[0], sin(1.5), ABS_TOL); // actual_sin/sin
    EXPECT_NEAR(values[2], sin(1.5), LOOSE_TOL); // deriv_approx_sin/sin
    csim_freeVector(values);
}

TEST(SBW, one_step_import) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    code = csim_setTolerances(1.0, 1.0, 10);
    code = csim_oneStep(1.5);
    EXPECT_EQ(code, 0);
    // check the outputs
    code = csim_getValues(&values, &length);
    EXPECT_NEAR(values[4], 1.5, ABS_TOL); // main/x
    EXPECT_NEAR(values[1], sin(1.5), ABS_TOL); // main/sin1 (actual sine)
    EXPECT_NEAR(values[2], sin(1.5), LOOSE_TOL); // main/sin2 (deriv approx)
    EXPECT_NEAR(values[3], sin(1.5), LOOSE_TOL); // main/sin3 (parabolic approx)
    csim_freeVector(values);
}

TEST(SBW, reset) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    // simulate a bit
    code = csim_setTolerances(1.0, 1.0, 10);
    code = csim_oneStep(1.5);
    // reset and check the outputs are back to the same
    code = csim_reset();
    EXPECT_EQ(code, 0);
    code = csim_getValues(&values, &length);
    EXPECT_NEAR(values[0], 0.0, ABS_TOL);
    EXPECT_NEAR(values[5], 0.0, ABS_TOL);
    EXPECT_NEAR(values[10], 0.75, ABS_TOL);
    csim_freeVector(values);
}

TEST(SBW, reset_import) {
    char* modelString;
    int length;
    int code = csim_serialiseCellmlFromUrl(
                TestResources::getLocation(
                    TestResources::CELLML_SINE_IMPORTS_MODEL_RESOURCE),
                &modelString, &length);
    // no point continuing if this fails
    ASSERT_EQ(code, 0);
    code = csim_loadCellml(modelString);
    ASSERT_EQ(code, 0);
    csim_freeVector(modelString);
    double* values;
    code = csim_setTolerances(1.0, 1.0, 10);
    code = csim_oneStep(1.5);
    // reset and check the outputs
    code = csim_reset();
    EXPECT_EQ(code, 0);
    code = csim_getValues(&values, &length);
    EXPECT_NEAR(values[1], 0.0, ABS_TOL);
    EXPECT_NEAR(values[3], 0.0, ABS_TOL);
    EXPECT_NEAR(values[4], 0.0, ABS_TOL);
    csim_freeVector(values);
}


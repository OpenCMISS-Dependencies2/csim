#include "gtest/gtest.h"

#include "csim/version.h"

TEST(Version, version_match) {
    unsigned int version = csim::version();
    EXPECT_EQ(0x000100, version);

    std::string version_string = csim::versionString();
    EXPECT_EQ("0.1.0", version_string);
}


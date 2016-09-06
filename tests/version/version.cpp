#include "gtest/gtest.h"

#include "csim/version.h"

TEST(Version, version_match) {
    unsigned int version = csim::version();
    EXPECT_EQ(0x020002, version);

    std::string version_string = csim::versionString();
    EXPECT_EQ("2.0.2", version_string);
}


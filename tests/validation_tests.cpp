#include <argparse.h>

#include "gtest/gtest.h"

TEST(ValidationTests, TestDoubleDashArgumentIsOptional)
{
    ASSERT_TRUE(argparse::validate::is_optional("--foo"));
}

TEST(ValidationTests, TestSingleDashArgumentIsOptional)
{
    ASSERT_TRUE(argparse::validate::is_optional("-foo"));
}

TEST(ValidationTests, TestPositionalArgumentIsNotOptional)
{
    ASSERT_FALSE(argparse::validate::is_optional("foo"));
}
#include <argparse.h>

#include "gtest/gtest.h"

// TODO: This test is incorrect - check the function name shouldnt be changed
TEST(ArgumentTests, TestArgumentLongestNameStringReturned)
{
    std::vector<std::string> argument_names = {"--myarg", "-m"};
    argparse::argument arg(argument_names);
    ASSERT_EQ(arg.get_longest_name_string(), "-m");
}
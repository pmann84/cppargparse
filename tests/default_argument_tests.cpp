#include <argparse.h>

#include "gtest/gtest.h"

TEST(DefaultArgumentTests, TestDefaultPositionalArgumentReturnedAndNoErrorWhenArgumentNotSubmitted)
{
    const std::string default_arg_value = "my BAR";
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser
        .add_argument("bar")
            .help("Positional bar argument.")
            .default_value(default_arg_value);

    std::vector<char*> argv = {"DummyApp.exe"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), default_arg_value);
}

// TODO: Test default values with nargs
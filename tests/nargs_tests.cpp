#include <argparse.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(NargsTests, TestArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
}

TEST(NargsTests, TestExitsWhenInsufficientMultiplePositionalArgumentsAreEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST(NargsTests, TestMultipleArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");
    parser.add_argument("bar").num_args(2).help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "BAR1", "BAR2"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
    ASSERT_THAT(parser.get<std::vector<std::string>>("bar"), ::testing::ContainerEq(std::vector<std::string>({"BAR1", "BAR2"})));
}

TEST(NargsTests, TestExitsWhenInsufficientArgumentsAreEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");
    parser.add_argument("bar").num_args(2).help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "BAR1"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST(NargsTests, TestAllPositionalArgsAreConsumedWhenOnlyPositionalArgumentsPresent)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("*").help("Positional bar argument consumes all.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "FOO4", "FOO5"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3", "FOO4", "FOO5"})));
}

TEST(NargsTests, TestEmptyVectorReturnedWhenConsumingAllArgumentsButNoInputsGiven)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("*").help("Positional bar argument consumes all.");

    std::vector<char*> argv = {"DummyApp.exe"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_TRUE(parser.get<std::vector<std::string>>("foo").empty());
}

TEST(NargsTests, TestAllPositionalArgsAreConsumedUpToNextOptionalArgument)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("*").help("Positional bar argument consumes all.");
    parser.add_argument({"-b", "--bar"}).help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "FOO4", "FOO5", "-b", "hello"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3", "FOO4", "FOO5"})));
    ASSERT_EQ(parser.get<std::string>("bar"), "hello");
}

TEST(NargsTests, TestAllPositionalArgsAreConsumedWithMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("*").help("Positional bar argument consumes all.");
    parser.add_argument("bar").num_args("*").help("Positional bar argument consumes all too!");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "FOO4", "FOO5"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3", "FOO4", "FOO5"})));
    ASSERT_TRUE(parser.get<std::string>("bar").empty());
}

TEST(NargsTests, TestUnknownPositionalArgumentRecievedGivingArgumentsAfterConsumeAllPositionalAndOptionalArgs)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("*").help("Positional bar argument consumes all.");
    parser.add_argument({"-b", "--bar"}).help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "FOO4", "FOO5", "-b", "hello", "NOPE"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST(NargsTests, TestSingleArgumentReturnsCorrectValueForPresentPositionalArgument)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("?").help("Positional bar argument consumes single if available.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1" };
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(argv[1], parser.get<std::string>("foo"));
}

TEST(NargsTests, TestSingleArgumentReturnsCorrectDefaultValueForPositionalArgumentWithNoValueProvidedAndNoDefaultProvided)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("?").help("Positional bar argument consumes single if available.");

    std::vector<char*> argv = {"DummyApp.exe"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ("", parser.get<std::string>("foo"));
}

TEST(NargsTests, TestSingleArgumentReturnsCorrectDefaultValueForPositionalArgumentWithNoValueProvided)
{
    const std::string default_arg_value = "my FOO";
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args("?").default_value(default_arg_value).help("Positional bar argument consumes single if available.");

    std::vector<char*> argv = {"DummyApp.exe"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(default_arg_value, parser.get<std::string>("foo"));
}


// Single arg Test cases

// Note that ? args only consume if there is an arg available to consume, other args take precedence in consuming arguments

// foo = ?, bar (1 arg) -> parse(["hello"]) => bar = hello, foo = default/none
// foo = ?, bar (1 arg) -> parse(["hello", "world"]) => bar = world, foo = hello
// bar (1 arg), foo = ? -> parse(["hello", "world"]) => bar = hello, foo = world (respects order)
// foo = ? -> parse(["hello", "world"]) => unrecognized arg "world"
// foo = ?, bar = ? -> parse(["hello", "world", "again"]) => unrecognized arg "again"


// At Least One Test Cases
// foo = + => parse([]) => Error generated - at least one arg expected
// foo = + => parse(["Hello"]) => foo = Hello
// foo = + => parse(["Hello", "World"]) => foo = [Hello, World]

// TODO: test integer nargs with optional arguments
// TODO: Test switches with narg 0
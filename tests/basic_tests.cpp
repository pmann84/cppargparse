#include <argparse.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(BasicTests, TestOnePositionalArgumentSuccessfullyEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
}

TEST(BasicTests, TestTwoPositionalArgumentsSuccessfullyEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument("foo").help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST(BasicTests, TestArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
}

TEST(BasicTests, TestMultipleArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");
    parser.add_argument("bar").num_args(2).help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "BAR1", "BAR2"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
    ASSERT_THAT(parser.get<std::vector<std::string>>("bar"), ::testing::ContainerEq(std::vector<std::string>({"BAR1", "BAR2"})));
}

// TODO: Throws bad any_cast...
//TEST(BasicTests, TestTwoPositionalArgumentsSuccessfullyEnteredGettingDifferentType)
//{
//    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
//    parser.add_argument("inputpath").help("Path to input file.");
//    parser.add_argument("timeout").help("A user defined timeout for something");
//
//    std::vector<char*> argv = {"DummyApp.exe", "myinputpath", "5"};
//    parser.parse_args(argv.size(), &argv[0]);
//    ASSERT_EQ(parser.get<std::string>("inputpath"), argv[1]);
//    ASSERT_EQ(parser.get<int>("timeout"), 5);
//}

TEST(BasicTests, TestOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-foo", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST(BasicTests, TestOptionalArgumentShortVersionEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[3]);
}

TEST(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfullyOptionalArgumentFirst)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO", "BAR"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[3]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST(BasicTests, TestMixedNameArgumentsThrowError)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    ASSERT_ANY_THROW(parser.add_argument({"foo", "-f"}));
}



/// Temporary test
TEST(BasicTests, Sandbox)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").help("foo argument help.");
    parser.add_argument("bar").num_args(3).help("bar argument help.");
    parser.add_argument({"-b", "--baz"}).help("baz argument help.");
    parser.add_argument({"-g", "--goo"}).num_args(3).help("baz argument help.");

    std::vector<char*> argv = {"DummyApp.exe", "a", "b1", "b2", "b3"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(true, true);
}
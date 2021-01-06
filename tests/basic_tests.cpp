//
// Created by Peter on 21/12/2020.
//

#include <argparse.h>

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

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
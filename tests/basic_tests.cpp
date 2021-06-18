#include <argparse.h>

#include "gtest/gtest.h"

class BasicTests: public ::testing::Test
{
public:
    BasicTests()
    {
    }

    void SetUp()
    {
        m_original_buffer = std::cout.rdbuf(nullptr);
    }

    void TearDown()
    {
        std::cout.rdbuf(m_original_buffer);
    }

private:
    std::basic_streambuf<char, std::char_traits<char>>* m_original_buffer;
};

TEST_F(BasicTests, TestOnePositionalArgumentSuccessfullyEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
}

TEST_F(BasicTests, TestOnePositionalArgumentNotEnteredExits)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST_F(BasicTests, TestTwoPositionalArgumentsSuccessfullyEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument("foo").help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestOnePositionalArgumentNotEnteredButTwoRequiredExits)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument("foo").help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST_F(BasicTests, TestThreePositionalArgumentEnteredButTwoRequiredExits)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument("foo").help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO", "BAR", "BAZ"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST_F(BasicTests, TestOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"--foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "--foo", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestOptionalArgumentShortVersionEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"--foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"--foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[3]);
}

TEST_F(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfullyOptionalArgumentFirst)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"--foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO", "BAR"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[3]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestMixedNameArgumentsThrowError)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    ASSERT_ANY_THROW(parser.add_argument({"foo", "-f"}));
}
#include <argparse.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

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

TEST_F(BasicTests, TestArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
}

TEST_F(BasicTests, TestExitsWhenInsufficientMultiplePositionalArgumentsAreEntered)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST_F(BasicTests, TestMultipleArgumentsAreReturnedForSuccessfullyEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");
    parser.add_argument("bar").num_args(2).help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "FOO3", "BAR1", "BAR2"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_THAT(parser.get<std::vector<std::string>>("foo"), ::testing::ContainerEq(std::vector<std::string>({"FOO1", "FOO2", "FOO3"})));
    ASSERT_THAT(parser.get<std::vector<std::string>>("bar"), ::testing::ContainerEq(std::vector<std::string>({"BAR1", "BAR2"})));
}

TEST_F(BasicTests, TestExitsWhenInsufficientArgumentsAreEnteredMultiplePositionalArguments)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("foo").num_args(3).help("Positional foo argument.");
    parser.add_argument("bar").num_args(2).help("Positional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "FOO1", "FOO2", "BAR1"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST_F(BasicTests, TestOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-foo", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestOptionalArgumentShortVersionEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
}

TEST_F(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfully)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

    std::vector<char*> argv = {"DummyApp.exe", "BAR", "-f", "FOO"};
    parser.parse_args(argv.size(), &argv[0]);
    ASSERT_EQ(parser.get<std::string>("bar"), argv[1]);
    ASSERT_EQ(parser.get<std::string>("foo"), argv[3]);
}

TEST_F(BasicTests, TestPositionalAndOptionalArgumentEnteredSuccessfullyOptionalArgumentFirst)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("bar").help("Positional bar argument.");
    parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

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

// TEST(BasicTests, TestAttemptToAccessArgumentValueThatHasNotBeenSpecified)
// {
//     auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
//     parser.add_argument("bar").help("Positional bar argument.");
//     parser.add_argument({"-foo", "-f"}).help("Optional Foo argument.");

//     std::vector<char*> argv = {"DummyApp.exe", "-f", "FOO", "BAR"};
//     parser.parse_args(argv.size(), &argv[0]);
//     ASSERT_EQ(parser.get<std::string>("bar"), argv[3]);
//     ASSERT_EQ(parser.get<std::string>("foo"), argv[2]);
// }
#include <argparse.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(ConfigFileTests, TestExitsWhenConfigFileFlagIsNotPresent)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.enable_config_file();
    parser.add_argument("foo").help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST(ConfigFileTests, TestExitsWhenConfigFileArgumentIsNotPresent)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.enable_config_file();
    parser.add_argument("foo").help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "-c"};
    EXPECT_EXIT(parser.parse_args(argv.size(), &argv[0]), testing::ExitedWithCode(1), "");
}

TEST(ConfigFileTests, TestConfigFileDoesNotExistThrowsError)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.enable_config_file();
    parser.add_argument("foo").help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "--configFile", "config_file_that_does_not_exist"};
    ASSERT_ANY_THROW(parser.parse_args(argv.size(), &argv[0]));
}

TEST(ConfigFileTests, TestConfigFileArgumentIsPopulatedCorrectly)
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.enable_config_file();
    parser.add_argument("foo").help("Optional bar argument.");

    std::vector<char*> argv = {"DummyApp.exe", "--configFile", "./test_config_files/test_empty_config_file.cfg"};
    parser.parse_args(argv.size(), &argv[0]);

    ASSERT_EQ("./test_config_files/test_empty_config_file.cfg", parser.get<std::string>("configFile"));
}

TEST(ConfigFileTests, TestConfigFileIsReadCorrectly)
{
    argparse::config_file_reader config_reader;
    auto config = config_reader.read_args("./test_config_files/simple_config_file.cfg");
    ASSERT_EQ("Hello", config["foo"]);
    ASSERT_EQ("World", config["bar"]);
}

TEST(ConfigFileTests, TestThrowsIfConfigIsInvalid)
{
    argparse::config_file_reader config_reader;
    EXPECT_ANY_THROW(config_reader.read_args("./test_config_files/invalid_config.cfg"));
}
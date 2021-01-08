#include <argparse.h>

void setup_simple_positional_arg(argparse::argument_parser& parser)
{
    parser.add_argument("foo").help("foo argument help.");
}

void setup_multiarg_example(argparse::argument_parser& parser)
{
    parser.add_argument("foo").help("foo argument help.");
    parser.add_argument("bar").num_args(3).help("bar argument help.");
    parser.add_argument({"-b", "--baz"}).help("baz argument help.");
    parser.add_argument({"-g", "--goo"}).num_args(3).help("goo argument help.");
}


int main(int argc, char *argv[])
{
    // Setup the parser
    auto parser = argparse::argument_parser("MyParser", "Parser Description");
    setup_multiarg_example(parser);
    // Parse the arguments
    parser.parse_args(argc, argv);
    // Get some values from the parser
    auto foo = parser.get<std::string>("foo");
    auto bar = parser.get<std::vector<std::string>>("bar");
    return 0;
}
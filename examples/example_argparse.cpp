#include <argparse.h>

void setup_parser(argparse::argument_parser& parser)
{
    parser.add_argument("foo").help("foo argument help.");
    parser.add_argument("bar").num_args(3).help("bar argument help.");
    parser.add_argument({"-b", "--baz"}).help("baz argument help.");
    parser.add_argument({"-g", "--goo"}).num_args(3).help("baz argument help.");
}

int main(int argc, char *argv[])
{
    auto p = argparse::argument_parser();
    setup_parser(p);
    p.parse_args(argc, argv);
    std::vector<std::string> bar_vals = p.get<std::vector<std::string>>("bar");
    std::cout << "bar has values: ";
    for (auto bv : bar_vals)
    {
        std::cout << bv << ", ";
    }
    return 0;
}
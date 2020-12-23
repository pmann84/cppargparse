//
// Created by Peter on 21/12/2020.
//

#include <argparse.h>

#include <iostream>

int main(int argc, char *argv[])
{
    auto parser = argparse::argument_parser("MyParser", "Commandline options for my application!");
    parser.add_argument("inputpath").help("Path to input file.");
    parser.add_argument("name").help("Name of something.");
    parser.add_argument({"-f", "--foo"}).help("Optional foo argument.");

    parser.parse_args(argc, argv);
//    std::cin.get();
    return 0;
}

#include <argparse.hpp>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("listuser");

    program.add_argument("user")
    .help("RACF user to list");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input = program.get<std::string>("user");

    return 0;
}
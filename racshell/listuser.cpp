
#include <argparse.hpp>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("listuser");

    // Main parameter, the user to list
    program.add_argument("user")
    .help("RACF user to list");

    // Optional parameters for data not displayed by default
    program.add_argument("-g", "--groups")
    .help("List connected RACF groups");

    program.add_argument("-t", "--tso")
    .help("List TSO segment");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input = program.get<std::string>("user");

    // Check if the input is too long to be a valid user
    if (input.length() > 8) {
        std::cout << std::string("\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF userid \n");
    } else {
        std::cout << std::string("\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n");
        std::cout << std::string("Listuser: ") << input << std::string("\n");
        std::cout << std::string("\033[35m--------------------------------------------------------------------\x1b[0m\n");
    }

    return 0;
}
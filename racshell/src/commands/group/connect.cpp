#include "include/racshell/commands/group_connection_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::GroupConnectionCommandSpec spec = {
        "connect",
        "alter",
        "RACF userid to connect",
        "RACF group to connect the user to",
        "connected to",
        true,
        "connection traits to set, e.g. base:special=true"};

    return racshell::run_group_connection_command(argc, argv, spec);
}
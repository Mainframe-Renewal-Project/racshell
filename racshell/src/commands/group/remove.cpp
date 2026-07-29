#include "include/racshell/commands/group_connection_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::GroupConnectionCommandSpec spec = {
        "remove",
        "delete",
        "RACF userid to remove from a group",
        "RACF group to remove the user from",
        "removed from",
        false,
        nullptr};

    return racshell::run_group_connection_command(argc, argv, spec);
}

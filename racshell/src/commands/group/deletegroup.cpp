#include "include/racshell/commands/delete_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::DeleteEntityCommandSpec spec = {
        "deletegroup",
        "group",
        "group",
        "RACF group to delete",
        "Invalid input, must be a valid RACF group name",
        "Group"
    };

    return racshell::run_delete_entity_command(argc, argv, spec);
}

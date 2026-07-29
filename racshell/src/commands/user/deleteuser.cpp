#include "include/racshell/commands/delete_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::DeleteEntityCommandSpec spec = {
        "deleteuser",
        "user",
        "userid",
        "RACF userid to delete",
        "Invalid input, must be a valid RACF userid",
        "User"
    };

    return racshell::run_delete_entity_command(argc, argv, spec);
}

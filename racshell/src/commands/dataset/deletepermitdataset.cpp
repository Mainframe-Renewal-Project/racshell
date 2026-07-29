#include "include/racshell/commands/permit_command.hpp"

int main(int argc, char *argv[])
{
    racshell::PermitCommandSpec spec{
        .command_name = "deletepermitdataset",
        .operation = "delete",
        .is_dataset = true,
        .allow_traits = false,
        .success_verb = "deleted"
    };

    return racshell::run_permit_command(argc, argv, spec);
}
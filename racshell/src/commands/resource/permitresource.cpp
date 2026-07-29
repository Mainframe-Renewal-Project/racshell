#include "include/racshell/commands/permit_command.hpp"

int main(int argc, char *argv[])
{
    racshell::PermitCommandSpec spec{
        .command_name = "permitresource",
        .operation = "alter",
        .is_dataset = false,
        .allow_traits = true,
        .success_verb = "created"
    };

    return racshell::run_permit_command(argc, argv, spec);
}
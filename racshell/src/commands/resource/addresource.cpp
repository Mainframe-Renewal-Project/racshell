#include "include/racshell/commands/resource_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::ResourceCommandSpec spec = {"addresource", "add", true, "Resource profile", "created"};
    return racshell::run_resource_command(argc, argv, spec);
}

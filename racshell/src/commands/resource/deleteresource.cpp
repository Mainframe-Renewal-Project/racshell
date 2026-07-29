#include "include/racshell/commands/resource_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::ResourceCommandSpec spec = {"deleteresource", "delete", false, "Resource profile", "deleted"};
    return racshell::run_resource_command(argc, argv, spec);
}

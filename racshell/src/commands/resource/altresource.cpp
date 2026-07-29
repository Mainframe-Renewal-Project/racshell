#include "include/racshell/commands/resource_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::ResourceCommandSpec spec = {"altresource", "alter", true, "Resource profile", "altered"};
    return racshell::run_resource_command(argc, argv, spec);
}

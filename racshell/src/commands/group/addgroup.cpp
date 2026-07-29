#include "include/racshell/commands/add_entity_command.hpp"

int main(int argc, char *argv[])
{
    const std::vector<racshell::TraitArg> trait_args = {
        {"-s", "--superior-group", "base:superior_group", "superior group for the new group"},
        {"-o", "--owner",          "base:owner",          "owner of the new group"},
        {"-g", "--omvs-gid",       "omvs:gid",            "z/OS Unix group ID (GID) for the new group"},
        {"-G", "--omvs-auto-gid",  "omvs:auto_gid",       "automatically assign a z/OS Unix GID to the new group", true},
    };

    const racshell::AddEntityCommandSpec spec = {
        .command_name = "addgroup",
        .admin_type = "group",
        .entity_argument = "group",
        .entity_help = "RACF group to create",
        .entity_validation_error = "Invalid input, must be a valid RACF group name",
        .traits_help = "traits to set, e.g. base:owner=SYS1 base:superior_group=SYS1",
        .success_label = "Group",
        .setup_extra_args = [trait_args](argparse::ArgumentParser &program)
        {
            racshell::register_trait_args(program, trait_args);
        },
        .apply_extra_args = [trait_args](argparse::ArgumentParser &program, nlohmann::json &request)
        {
            racshell::apply_trait_args(program, request, trait_args);
        }};

    return racshell::run_add_entity_command(argc, argv, spec);
}

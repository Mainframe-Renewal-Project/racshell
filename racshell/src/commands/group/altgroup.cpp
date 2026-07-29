#include "include/racshell/commands/alter_entity_command.hpp"

int main(int argc, char *argv[])
{
    const std::vector<racshell::TraitArg> trait_args = {
        {"-s", "--superior-group", "base:superior_group", "superior group for the group"},
        {"-o", "--owner",          "base:owner",          "owner of the group"},
        {"-g", "--omvs-gid",       "omvs:gid",            "z/OS Unix group ID (GID) for the group"},
        {"-G", "--omvs-auto-gid",  "omvs:auto_gid",       "automatically assign a z/OS Unix GID to the group", true},
    };

    const racshell::AlterEntityCommandSpec spec = {
        .command_name = "altgroup",
        .admin_type = "group",
        .entity_argument = "group",
        .entity_help = "RACF group to alter",
        .entity_validation_error = "Invalid input, must be a valid RACF group",
        .traits_help = "traits to alter, e.g. base:owner='SYSPROG' omvs:gid=1234567",
        .success_label = "Group",
        .setup_extra_args = [trait_args](argparse::ArgumentParser &program)
        {
            racshell::register_trait_args(program, trait_args);
        },
        .apply_extra_args = [trait_args](argparse::ArgumentParser &program, nlohmann::json &request)
        {
            racshell::apply_trait_args(program, request, trait_args);
        }};

    return racshell::run_alter_entity_command(argc, argv, spec);
}

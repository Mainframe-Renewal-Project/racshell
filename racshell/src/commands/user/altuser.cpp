#include "include/racshell/commands/alter_entity_command.hpp"

int main(int argc, char *argv[])
{
    const std::vector<racshell::TraitArg> trait_args = {
        {"-n", "--name",           "base:name",           "full name of the user"},
        {"-s", "--special",        "base:special",        "special user flag, e.g. true or false"},
        {"-o", "--omvs-uid",       "omvs:uid",            "z/OS Unix user ID (UID) for the user"},
        {"-d", "--omvs-home-dir",  "omvs:home_directory", "z/OS Unix home directory for the user"},
        {"-G", "--omvs-auto-uid",  "omvs:auto_uid",       "automatically assign a z/OS Unix UID to the user", true},
    };

    const racshell::AlterEntityCommandSpec spec = {
        .command_name = "altuser",
        .admin_type = "user",
        .entity_argument = "userid",
        .entity_help = "RACF userid to alter",
        .entity_validation_error = "Invalid input, must be a valid RACF userid",
        .traits_help = "traits to alter, e.g. base:name='LEONARD CARCARAMO' base:special=true",
        .success_label = "User",
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

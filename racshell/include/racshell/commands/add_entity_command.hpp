#pragma once

#include "include/racshell/commands/entity_command.hpp"

namespace racshell
{

    struct AddEntityCommandSpec
    {
        const char *command_name;
        const char *admin_type;
        const char *entity_argument;
        const char *entity_help;
        const char *entity_validation_error;
        const char *traits_help;
        const char *success_label;
        std::function<void(argparse::ArgumentParser &)> setup_extra_args = nullptr;
        std::function<void(argparse::ArgumentParser &, nlohmann::json &)> apply_extra_args = nullptr;
    };

    inline int run_add_entity_command(
        int argc,
        char *argv[],
        const AddEntityCommandSpec &spec)
    {
        const EntityCommandSpec entity_spec = {
            .command_name = spec.command_name,
            .operation = "add",
            .success_verb = "created",
            .admin_type = spec.admin_type,
            .entity_argument = spec.entity_argument,
            .entity_help = spec.entity_help,
            .entity_validation_error = spec.entity_validation_error,
            .supports_traits = true,
            .traits_help = spec.traits_help,
            .success_label = spec.success_label,
            .max_name_length = 8,
            .setup_extra_args = spec.setup_extra_args,
            .apply_extra_args = spec.apply_extra_args};

        return run_entity_command(argc, argv, entity_spec);
    }

} // namespace racshell

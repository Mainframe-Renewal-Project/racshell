#pragma once

#include "include/racshell/commands/entity_command.hpp"

namespace racshell
{

    struct DeleteEntityCommandSpec
    {
        const char *command_name;
        const char *admin_type;
        const char *entity_argument;
        const char *entity_help;
        const char *entity_validation_error;
        const char *success_label;
    };

    inline int run_delete_entity_command(int argc, char *argv[], const DeleteEntityCommandSpec &spec)
    {
        const EntityCommandSpec entity_spec = {
            .command_name = spec.command_name,
            .operation = "delete",
            .success_verb = "deleted",
            .admin_type = spec.admin_type,
            .entity_argument = spec.entity_argument,
            .entity_help = spec.entity_help,
            .entity_validation_error = spec.entity_validation_error,
            .supports_traits = false,
            .success_label = spec.success_label,
            .max_name_length = 8};

        return run_entity_command(argc, argv, entity_spec);
    }

} // namespace racshell
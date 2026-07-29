#include "include/racshell/commands/dataset_command.hpp"

int main(int argc, char *argv[])
{
    racshell::DatasetCommandSpec spec{
        .command_name = "deletedataset",
        .operation = "delete",
        .success_verb = "deleted",
        .success_label = "Dataset profile",
        .dataset_help =
            "dataset profile name to delete, e.g. LEONARD.LIB.HLASM",
        .allow_generic = true
    };

    return racshell::run_dataset_command(argc, argv, spec);
}
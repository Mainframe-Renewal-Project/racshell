#include "include/racshell/commands/dataset_command.hpp"

int main(int argc, char *argv[])
{
    racshell::DatasetCommandSpec spec{
        .command_name = "adddataset",
        .operation = "add",
        .success_verb = "created",
        .success_label = "Dataset profile",
        .dataset_help =
            "dataset profile name to create, e.g. ESWIFT.TEST.**",
        .traits_help =
            "traits to set, e.g. base:universal_access=None base:owner=eswift",
        .allow_traits = true,
        .allow_generic = true};

    return racshell::run_dataset_command(argc, argv, spec);
}
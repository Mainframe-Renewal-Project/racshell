#include "include/racshell/commands/dataset_command.hpp"

int main(int argc, char *argv[])
{
    racshell::DatasetCommandSpec spec{
        .command_name = "altdataset",
        .operation = "alter",
        .success_verb = "altered",
        .success_label = "Dataset profile",
        .dataset_help =
            "dataset profile name to alter, e.g. ESWIFT.TEST.DATA",
        .traits_help =
            "traits to alter, e.g. base:owner=eswift",
        .allow_traits = true
    };

    return racshell::run_dataset_command(argc, argv, spec);
}
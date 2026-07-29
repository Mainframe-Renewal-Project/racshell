#include "include/racshell/commands/search_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::SearchCommandSpec spec = {
        .command_name = "searchdatasets",
        .admin_type = "dataset",
        .filter_key = "dataset_filter",
        .filter_help = "filter dataset profiles by name pattern, e.g. LEONARD",
        .filter_validation_error = "Invalid input, filter must not exceed 44 characters",
        .filter_max_length = 44,
        .empty_results_message = "No datasets found",
        .results_label = "Datasets"};

    return racshell::run_search_command(argc, argv, spec);
}

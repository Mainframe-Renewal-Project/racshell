#include "include/racshell/commands/search_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::SearchCommandSpec spec = {
        .command_name = "searchgroups",
        .admin_type = "group",
        .filter_key = "group_filter",
        .filter_help = "filter group profiles by group name pattern, e.g. SYS",
        .filter_validation_error = "Invalid input, filter must not exceed 8 characters",
        .filter_max_length = 8,
        .empty_results_message = "No groups found",
        .results_label = "Groups"};

    return racshell::run_search_command(argc, argv, spec);
}

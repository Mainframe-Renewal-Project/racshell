#include "include/racshell/commands/search_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::SearchCommandSpec spec = {
        .command_name = "searchusers",
        .admin_type = "user",
        .filter_key = "userid_filter",
        .filter_help = "filter user profiles by userid pattern, e.g. IBM",
        .filter_validation_error = "Invalid input, filter must not exceed 8 characters",
        .filter_max_length = 8,
        .empty_results_message = "No users found",
        .results_label = "Users"};

    return racshell::run_search_command(argc, argv, spec);
}

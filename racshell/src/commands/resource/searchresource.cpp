#include <string>
#include "include/racshell/commands/search_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::SearchCommandSpec spec = {
        .command_name = "searchresource",
        .admin_type = "resource",
        .filter_key = "resource_filter",
        .filter_help = "filter resource profiles by name pattern, e.g. IRR",
        .filter_validation_error = "Invalid input, filter must not exceed 44 characters",
        .filter_max_length = 44,
        .empty_results_message = "No resource profiles found",
        .results_label = "Resource profiles",
        .setup_extra_args = [](argparse::ArgumentParser &program)
        {
            program.add_argument("class")
                .help("resource profile class, e.g. facility");
        },
        .validate_extra_args = [](const argparse::ArgumentParser &program, std::string &error_message)
        {
            const std::string resource_class = program.get<std::string>("class");
            if (resource_class.empty() || resource_class.length() > 8)
            {
                error_message = "Invalid input, resource profile class must not exceed 8 characters";
                return false;
            }
            return true;
        },
        .apply_extra_args = [](const argparse::ArgumentParser &program, nlohmann::json &request)
        {
            request["class"] = racshell::to_racf_resource_class_text(program.get<std::string>("class"));
        },
        .build_text_header = [](const argparse::ArgumentParser &program, const std::string &filter)
        {
            const std::string resource_class = program.get<std::string>("class");
            return "Resource profiles in class '" + resource_class + "'" +
                (filter.empty() ? "" : " matching '" + filter + "'") + ":";
        }};

    return racshell::run_search_command(argc, argv, spec);
}

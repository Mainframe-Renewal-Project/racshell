#include <iostream>
#include <string>

#include "include/racshell/command_helper.hpp"
#include "include/racshell/output_formatter.hpp"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("rrsflist");
    program.add_description("Display RRSF configuration and nodes (read-only).");
    racshell::add_toggle_argument(program, "-N", "--nodes", "display full node details");
    racshell::add_toggle_argument(program, "-s", "--settings", "display direction notification and output settings");
    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &error)
    {
        racshell::print_error(std::cerr, error.what());
        std::cerr << program;
        return 1;
    }

    racshell::set_color_output_enabled(!program.get<bool>("no-color"));
    const nlohmann::json request = {{"operation", "extract"}, {"admin_type", "racf-rrsf"}};
    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), program.get<bool>("debug"));
    if (result == nullptr || result->result_json == nullptr)
    {
        racshell::print_error(std::cerr, "No response returned by SEAR");
        return 1;
    }

    const bool all_json = program.get<bool>("all-json");
    if (all_json)
        std::cout << result->result_json << "\n";
    const auto operation = racshell::validate_sear_operation_result(result->result_json);
    if (!operation.success)
    {
        racshell::print_error(std::cerr, operation.error_message);
        racshell::print_sear_errors(operation.response, std::cerr);
        return 1;
    }
    const auto response = racshell::validate_sear_response(result->result_json, "RRSF configuration");
    if (!response.success)
    {
        racshell::print_error(std::cerr, response.error_message);
        return 1;
    }
    if (all_json)
        return 0;

    RrsfData data{response.profile};
    auto &base = data.profile["base"];
    if (!program.get<bool>("settings"))
    {
        for (auto field = base.begin(); field != base.end();)
        {
            const auto &key = field.key();
            if (key.size() >= 9 && key.compare(key.size() - 9, 9, "_settings") == 0)
                field = base.erase(field);
            else
                ++field;
        }
    }
    if (!program.get<bool>("nodes") && base.contains("base:nodes") && base["base:nodes"].is_array())
    {
        for (auto &node : base["base:nodes"])
        {
            nlohmann::json summary = nlohmann::json::object();
            for (const auto *key : {"base:node_name", "base:node_description", "base:is_local_node",
                                    "base:node_state", "base:node_protocol", "base:is_multisystem_node", "base:is_main_node"})
            {
                if (node.contains(key))
                    summary[key] = node[key];
            }
            node = summary;
        }
    }
    if (program.get<bool>("json"))
        std::cout << JsonFormatter().format(data);
    else
        std::cout << TextFormatter().format(data);
    std::cout << "\n";
    return 0;
}
#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"

namespace racshell
{

    struct ResourceCommandSpec
    {
        const char *command_name;
        const char *operation;
        bool allow_traits;
        const char *success_label;
        const char *success_verb;
    };

    inline int run_resource_command(int argc, char *argv[], const ResourceCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        program.add_argument("resource")
            .help("resource profile name, e.g. IRR.RADMIN.**");

        program.add_argument("class")
            .help("resource profile class, e.g. facility");

        if (spec.allow_traits)
        {
            program.add_argument("-t", "--traits")
                .help("traits to set or alter, e.g. base:owner=SECADM base:universal_access=None")
                .nargs(argparse::nargs_pattern::any);
        }

        add_no_color_argument(program);
        add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
        add_toggle_argument(program, "-j", "--json", "output as JSON");
        add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

        try
        {
            program.parse_args(argc, argv);
        }
        catch (const std::exception &err)
        {
            print_error(std::cerr, err.what());
            std::cerr << program;
            return 1;
        }

        set_color_output_enabled(!program.get<bool>("no-color"));

        const std::string resource = program.get<std::string>("resource");
        if (resource.empty() || resource.length() > 44)
        {
            print_error(std::cerr, "Invalid input, resource profile name must not exceed 44 characters");
            return 1;
        }

        const std::string resource_class = program.get<std::string>("class");
        if (resource_class.empty() || resource_class.length() > 8)
        {
            print_error(std::cerr, "Invalid input, resource class must not exceed 8 characters");
            return 1;
        }

        const std::string request_resource = to_racf_identifier_text(resource);
        const std::string request_resource_class = to_racf_identifier_text(resource_class);

        const bool debug = program.get<bool>("debug");
        const bool json_output = program.get<bool>("json");
        const bool all_json = program.get<bool>("all-json");

        nlohmann::json traits = nlohmann::json::object();
        if (spec.allow_traits)
        {
            if (!parse_traits(program.get<std::vector<std::string>>("traits"), traits))
            {
                return 1;
            }
        }

        nlohmann::json request = {
            {"operation", spec.operation},
            {"admin_type", "resource"},
            {"resource", request_resource},
            {"class", request_resource_class}};

        if (spec.allow_traits)
        {
            request["traits"] = traits;
        }

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        const SearOperationInfo op_info = validate_sear_operation_result(result->result_json);
        if (!op_info.success)
        {
            print_error(std::cerr, op_info.error_message);
            print_sear_errors(op_info.response, std::cerr);
            return 1;
        }

        if (json_output)
        {
            nlohmann::json output;
            output["resource"] = resource;
            output["class"] = resource_class;
            output["status"] = spec.success_verb;
            std::cout << output.dump(2) << "\n";
        }
        else
        {
            print_success_prefix(std::cout);
            std::cout << spec.success_label << " " << resource << " " << spec.success_verb << " successfully in class " << resource_class << ".\n";
        }
        return 0;
    }

} // namespace racshell

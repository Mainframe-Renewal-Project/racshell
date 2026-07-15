#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <cstddef>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"

namespace racshell
{

    struct EntityCommandSpec
    {
        const char *command_name;
        const char *operation;
        const char *success_verb;
        const char *admin_type;
        const char *entity_argument;
        const char *entity_help;
        const char *entity_validation_error;
        bool supports_traits;
        const char *traits_help;
        const char *success_label;
        std::size_t max_name_length;
        std::function<void(argparse::ArgumentParser &)> setup_extra_args = nullptr;
        std::function<void(argparse::ArgumentParser &, nlohmann::json &)> apply_extra_args = nullptr;
    };

    inline int run_entity_command( int argc, char *argv[], const EntityCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        program.add_argument(spec.entity_argument)
            .help(spec.entity_help);

        if (spec.supports_traits)
        {
            program.add_argument("-t", "--traits")
                .help(spec.traits_help)
                .nargs(argparse::nargs_pattern::any);
        }

        if (spec.setup_extra_args)
        {
            spec.setup_extra_args(program);
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

        const std::string display_entity_value = program.get<std::string>(spec.entity_argument);
        if (display_entity_value.length() > spec.max_name_length)
        {
            print_error(std::cerr, spec.entity_validation_error);
            return 1;
        }
        const std::string request_entity_value = to_racf_identifier_text(display_entity_value);

        const bool debug = program.get<bool>("debug");
        const bool json_output = program.get<bool>("json");
        const bool all_json = program.get<bool>("all-json");

        nlohmann::json request = {
            {"operation", spec.operation},
            {"admin_type", spec.admin_type},
            {spec.entity_argument, request_entity_value}};

        if (spec.supports_traits)
        {
            nlohmann::json traits;

            if (!parse_traits( program.get<std::vector<std::string>>("traits"), traits))
            {
                return 1;
            }

            request["traits"] = traits;
        }

        if (spec.apply_extra_args)
        {
            spec.apply_extra_args(program, request);
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
            output[spec.entity_argument] = display_entity_value;
            output["status"] = spec.success_verb;

            std::cout << output.dump(2) << "\n";
        }
        else
        {
            print_success_prefix(std::cout);
            std::cout << spec.success_label << " " << display_entity_value << " " << spec.success_verb << " successfully.\n";
        }
        return 0;
    }

} // namespace racshell
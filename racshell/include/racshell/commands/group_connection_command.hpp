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

    struct GroupConnectionCommandSpec
    {
        const char *command_name;
        const char *operation;
        const char *userid_help;
        const char *group_help;
        const char *success_message; // e.g. "connected to" or "removed from"
        bool with_traits;
        const char *traits_help;     // ignored when with_traits is false
    };

    inline int run_group_connection_command(int argc, char *argv[], const GroupConnectionCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        program.add_argument("userid")
            .help(spec.userid_help);

        program.add_argument("group")
            .help(spec.group_help);

        if (spec.with_traits)
        {
            program.add_argument("-t", "--traits")
                .help(spec.traits_help)
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

        const std::string userid = program.get<std::string>("userid");
        if (userid.length() > 8)
        {
            print_error(std::cerr, "Invalid input, must be a valid RACF userid");
            return 1;
        }

        const std::string group = program.get<std::string>("group");
        if (group.length() > 8)
        {
            print_error(std::cerr, "Invalid input, must be a valid RACF group");
            return 1;
        }

        const std::string request_userid = to_racf_identifier_text(userid);
        const std::string request_group = to_racf_identifier_text(group);

        const bool debug = program.get<bool>("debug");
        const bool json_output = program.get<bool>("json");
        const bool all_json = program.get<bool>("all-json");

        nlohmann::json request = {
            {"operation", spec.operation},
            {"admin_type", "group-connection"},
            {"userid", request_userid},
            {"group", request_group}};

        if (spec.with_traits)
        {
            nlohmann::json traits;
            if (!parse_traits(program.get<std::vector<std::string>>("traits"), traits))
            {
                return 1;
            }
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
            output["userid"] = userid;
            output["group"] = group;
            output["status"] = spec.success_message;
            std::cout << output.dump(2) << "\n";
        }
        else
        {
            print_success_prefix(std::cout);
            std::cout << "User " << userid << " " << spec.success_message << " group " << group << " successfully.\n";
        }
        return 0;
    }

} // namespace racshell

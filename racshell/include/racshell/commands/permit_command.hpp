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

    struct PermitCommandSpec
    {
        const char *command_name;
        const char *operation;

        bool is_dataset;
        bool allow_traits;

        const char *success_verb;
    };

    inline int run_permit_command(
        int argc,
        char *argv[],
        const PermitCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        if (spec.is_dataset)
        {
            program.add_argument("dataset")
                .help("dataset profile name, e.g. MATRIX.SECRETS.**");

            add_toggle_argument(program, "-g", "--generic", "treat the dataset name as a generic profile");

            program.add_argument("--volume")
                .help("dataset volume name");
        }
        else
        {
            program.add_argument("resource")
                .help("resource profile name, e.g. IRR.IRRSMO00.PRECHECK");

            program.add_argument("class")
                .help("resource profile class, e.g. XFACILIT");
        }

        program.add_argument("userid")
            .help("user ID receiving the permit");

        if (spec.allow_traits)
        {
            program.add_argument("-t", "--traits")
                .help("permit traits, e.g. base:access=READ")
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

        const bool debug = program.get<bool>("debug");
        const bool json_output = program.get<bool>("json");
        const bool all_json = program.get<bool>("all-json");

        const std::string userid = program.get<std::string>("userid");
        const std::string request_userid = to_racf_identifier_text(userid);

        nlohmann::json request = {
            {"operation", spec.operation},
            {"admin_type", "permission"},
            {"userid", request_userid}};

        if (spec.is_dataset)
        {
            const std::string dataset = program.get<std::string>("dataset");

            if (dataset.length() > 44)
            {
                print_error(std::cerr, "Invalid input, dataset profile name must not exceed 44 characters");
                return 1;
            }

            request["dataset"] = to_racf_identifier_text(dataset);

            if (program.get<bool>("generic"))
            {
                request["generic"] = true;
            }

            if (program.present("--volume")) // TODO: figure out how to this
            {
                request["volume"] =
                    program.get<std::string>("volume");
            }
        }
        else
        {
            request["resource"] =
                to_racf_identifier_text(program.get<std::string>("resource"));

            request["class"] =
                to_racf_identifier_text(program.get<std::string>("class"));
        }

        if (spec.allow_traits)
        {
            nlohmann::json traits;

            if (!parse_traits(
                    program.get<std::vector<std::string>>("traits"),
                    traits))
            {
                return 1;
            }

            request["traits"] = traits;
        }

        const std::string request_json = request.dump();

        sear_result_t *result =
            sear(request_json.c_str(),
                 request_json.length(),
                 debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        const SearOperationInfo op_info =
            validate_sear_operation_result(result->result_json);

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

            output["status"] =
                spec.success_verb;

            std::cout << output.dump(2) << "\n";
        }
        else
        {
            print_success_prefix(std::cout);

            std::cout
                << "Permit for "
                << userid
                << " "
                << spec.success_verb
                << " successfully.\n";
        }

        return 0;
    }

} // namespace racshell
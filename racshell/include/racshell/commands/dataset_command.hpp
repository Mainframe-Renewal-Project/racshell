#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/commands/entity_command.hpp"

namespace racshell
{

struct DatasetCommandSpec
{
    const char *command_name;
    const char *operation;

    const char *success_verb;
    const char *success_label;

    const char *dataset_help;
    const char *traits_help;

    bool allow_traits = false;
    bool allow_generic = false;
};


inline int run_dataset_command(
    int argc,
    char *argv[],
    const DatasetCommandSpec &spec)
{
    argparse::ArgumentParser program(spec.command_name);

    program.add_argument("dataset")
        .help(spec.dataset_help);


    if (spec.allow_traits)
    {
        program.add_argument("-t", "--traits")
            .help(spec.traits_help)
            .nargs(argparse::nargs_pattern::any);
    }

    if (spec.allow_generic)
    {
        add_toggle_argument(program, "-g", "--generic", "treat the dataset name as a generic profile");
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


    const std::string dataset = program.get<std::string>("dataset");


    if (dataset.length() > 44)
    {
        print_error( std::cerr, "Invalid input, dataset profile name must not exceed 44 characters");
        return 1;
    }

    const std::string request_dataset = to_racf_identifier_text(dataset);

    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");


    nlohmann::json request = {
        {"operation", spec.operation},
        {"admin_type", "dataset"},
        {"dataset", request_dataset}
    };


    if (spec.allow_traits)
    {
        nlohmann::json traits;

        if (!parse_traits(program.get<std::vector<std::string>>("traits"), traits))
        {
            return 1;
        }

        request["traits"] = traits;
    }


    if (spec.allow_generic &&
        program.get<bool>("generic"))
    {
        request["generic"] = true;
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

        output["dataset"] = dataset;
        output["status"] = spec.success_verb;

        std::cout << output.dump(2) << "\n";
    }
    else
    {
        print_success_prefix(std::cout);

        std::cout << spec.success_label << " " << dataset << " " << spec.success_verb << " successfully.\n";
    }
    return 0;
}


} // namespace racshell
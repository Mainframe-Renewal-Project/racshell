#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "include/racshell/output_formatter.hpp"
#include "include/racshell/command_helper.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("listdataset");

    program.add_argument("dataset")
        .help("dataset profile name to display, e.g. LEONARD.LIB.HLASM");

    racshell::add_toggle_argument(program, "-g", "--generic", "treat the dataset name as a generic profile");
    racshell::add_toggle_argument(program, "-x", "--csdata", "list CSDATA segment");
    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        racshell::print_error(std::cerr, err.what());
        std::cerr << program;
        return 1;
    }

    racshell::set_color_output_enabled(!program.get<bool>("no-color"));

    const std::string dataset = program.get<std::string>("dataset");
    if (dataset.length() > 44)
    {
        racshell::print_error(std::cerr, "Invalid input, dataset profile name must not exceed 44 characters");
        return 1;
    }
    const std::string request_dataset = racshell::to_racf_identifier_text(dataset);

    const bool generic = program.get<bool>("generic");
    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");
    const bool csdata = program.get<bool>("csdata");

    nlohmann::json request = {
        {"operation", "extract"},
        {"admin_type", "dataset"},
        {"dataset", request_dataset}};

    if (generic)
    {
        request["generic"] = true;
    }

    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

    if (all_json)
    {
        std::cout << result->result_json << "\n";
        return 0;
    }

    const racshell::SearResponseInfo sear_info = racshell::validate_sear_response(result->result_json, "dataset");
    if (!sear_info.success)
    {
        racshell::print_error(std::cerr, sear_info.error_message);
        return 1;
    }

    const nlohmann::json base = sear_info.base;
    const nlohmann::json profile = sear_info.profile;

    DatasetData dataset_data;
    dataset_data.dataset = dataset;

    racshell::assign_string(base, "base:owner", dataset_data.owner);
    racshell::assign_string(base, "base:universal_access", dataset_data.uacc);
    racshell::assign_bool(base, "base:audited", dataset_data.audited);

    if (base.contains("base:access_count") && base["base:access_count"].is_number())
    {
        dataset_data.access_count = base["base:access_count"].get<int>();
    }

    if (base.contains("base:access_list") && base["base:access_list"].is_array())
    {
        const auto &access_list = base["base:access_list"];
        for (const auto &entry : access_list)
        {
            AccessEntry access_entry;
            if (entry.is_object())
            {
                if (entry.contains("base:access_type") && entry["base:access_type"].is_string())
                {
                    access_entry.access_type = entry["base:access_type"].get<std::string>();
                }
                if (entry.contains("base:access_id") && entry["base:access_id"].is_string())
                {
                    access_entry.access_id = entry["base:access_id"].get<std::string>();
                }
            }
            else if (entry.is_string())
            {
                access_entry.access_type = entry.get<std::string>();
            }
            dataset_data.access_list.push_back(access_entry);
        }
    }

    if (csdata && profile.contains("csdata") && profile["csdata"].is_object())
    {
        dataset_data.csdata = profile["csdata"];
    }

    std::unique_ptr<OutputFormatter> formatter;
    if (json_output)
    {
        formatter = std::make_unique<JsonFormatter>();
    }
    else
    {
        formatter = std::make_unique<TextFormatter>();
    }

    std::cout << formatter->format(dataset_data) << "\n";

    return 0;
}

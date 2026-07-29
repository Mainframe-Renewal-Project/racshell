#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "include/racshell/output_formatter.hpp"
#include "include/racshell/command_helper.hpp"
#include "include/racshell/data/resource_data.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("listresource");

    program.add_argument("resource")
        .help("resource profile name, e.g. IRR.RADMIN.**");

    program.add_argument("class")
        .help("resource profile class, e.g. facility");

    // Optional parameters for data not displayed by default
    racshell::add_toggle_argument(program, "-x", "--csdata", "list CSDATA segment");
    racshell::add_toggle_argument(program, "-l", "--access", "list access control list");
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

    const std::string display_resource = program.get<std::string>("resource");
    if (display_resource.empty() || display_resource.length() > 44)
    {
        racshell::print_error(std::cerr, "Invalid input, resource profile name must not exceed 44 characters");
        return 1;
    }

    const std::string display_resource_class = program.get<std::string>("class");
    if (display_resource_class.empty() || display_resource_class.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, resource profile class must not exceed 8 characters");
        return 1;
    }

    const std::string request_resource = racshell::to_racf_identifier_text(display_resource);
    const std::string request_resource_class = racshell::to_racf_resource_class_text(display_resource_class);

    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");
    const bool csdata = program.get<bool>("csdata");
    const bool access = program.get<bool>("access");

    nlohmann::json request = {
        {"operation", "extract"},
        {"admin_type", "resource"},
        {"resource", request_resource},
        {"class", request_resource_class}};

    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

    if (all_json)
    {
        std::cout << result->result_json << "\n";
        return 0;
    }

    const racshell::SearResponseInfo sear_info = racshell::validate_sear_response(result->result_json, "resource");
    if (!sear_info.success)
    {
        racshell::print_error(std::cerr, sear_info.error_message);
        return 1;
    }

    ResourceData resource_data;
    resource_data.resource = display_resource;
    resource_data.resource_class = display_resource_class;
    resource_data.base = sear_info.base;
    resource_data.profile = sear_info.profile;
    racshell::assign_string(sear_info.base, "base:owner", resource_data.owner);
    resource_data.owner = racshell::from_racf_identifier_text(resource_data.owner);
    racshell::assign_string(sear_info.base, "base:universal_access", resource_data.uacc);
    
    if (csdata && sear_info.profile.contains("csdata") && sear_info.profile["csdata"].is_object())
    {
        resource_data.csdata = sear_info.profile["csdata"];
    }

    if (access && sear_info.profile.contains("access_list") && sear_info.profile["access_list"].is_array())
    {
        for (const auto &entry : sear_info.profile["access_list"])
        {
            AccessEntry access_entry;
            if (entry.contains("access_type"))
            {
                access_entry.access_type = entry["access_type"];
            }
            if (entry.contains("access_id"))
            {
                access_entry.access_id = racshell::from_racf_identifier_text(entry["access_id"]);
            }
            resource_data.access_list.push_back(access_entry);
        }
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

    std::cout << formatter->format(resource_data) << "\n";
    return 0;
}

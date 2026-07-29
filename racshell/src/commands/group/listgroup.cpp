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
    argparse::ArgumentParser program("listgroup");

    program.add_argument("group")
        .help("RACF group to list");

    racshell::add_toggle_argument(program, "-u", "--users", "list connected users");
    racshell::add_toggle_argument(program, "-x", "--csdata", "list CSDATA segment");
    racshell::add_toggle_argument(program, "-o", "--omvs", "list OMVS segment");
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

    const std::string input = program.get<std::string>("group");

    if (input.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group name");
        return 1;
    }
    else
    {

        bool debug = program.get<bool>("debug");
        bool users = program.get<bool>("users");
        bool omvs = program.get<bool>("omvs");
        bool json_output = program.get<bool>("json");
        bool all_json = program.get<bool>("all-json");
        bool csdata = program.get<bool>("csdata");
        const std::string request_group = racshell::to_racf_identifier_text(input);

        nlohmann::json req = {
            {"operation", "extract"},
            {"admin_type", "group"},
            {"group", request_group}};

        std::string request_json = req.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        racshell::SearResponseInfo sear_info = racshell::validate_sear_response(result->result_json, "group");
        if (!sear_info.success)
        {
            racshell::print_error(std::cerr, sear_info.error_message);
            return 1;
        }
        nlohmann::json base = sear_info.base;
        nlohmann::json profile = sear_info.profile;

        GroupData group_data;
        group_data.groupid = input;

        racshell::assign_string(base, "base:owner", group_data.owner);
        racshell::assign_string(base, "base:create_date", group_data.created_date);
        racshell::assign_string(base, "base:superior_group", group_data.superior_group);
        racshell::assign_string(base, "base:installation_data", group_data.installation_data);
        racshell::assign_bool(base, "base:universal", group_data.universal);
        racshell::assign_bool(base, "base:terminal_universal_access", group_data.terminal_universal_access);

        if (users && base.contains("base:connected_users") && base["base:connected_users"].is_array())
        {
            for (const auto &u : base["base:connected_users"])
            {
                GroupUser gu;
                racshell::assign_string(u, "base:connected_userid", gu.userid);
                racshell::assign_string(u, "base:connected_user_authority", gu.authority);
                group_data.connected_users.push_back(gu);
            }
        }

        if (omvs && profile.contains("omvs"))
        {
            group_data.omvs = profile["omvs"];
        }

        if (csdata && profile.contains("csdata") && profile["csdata"].is_object())
        {
            group_data.csdata = profile["csdata"];
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

        std::cout << formatter->format(group_data) << "\n";
    }
    return 0;
}

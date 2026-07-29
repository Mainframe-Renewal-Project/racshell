#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "include/racshell/command_helper.hpp"
#include "racshell/output_formatter.hpp"

namespace
{
    void add_connected_user_differences(const nlohmann::json &left_base,
                                        const nlohmann::json &right_base,
                                        nlohmann::json &differences);

    GroupComparisonData build_group_comparison_data(const std::string &left_groupid,
                                                    const std::string &right_groupid,
                                                    bool compare_users,
                                                    bool compare_omvs,
                                                    bool compare_csdata,
                                                    bool debug,
                                                    bool raw_json_output)
    {
        GroupComparisonData comparison;
        comparison.left.groupid = left_groupid;
        comparison.right.groupid = right_groupid;
        comparison.compare_users = compare_users;
        comparison.compare_omvs = compare_omvs;
        comparison.compare_csdata = compare_csdata;
        comparison.raw_json_output = raw_json_output;

        racshell::ExtractPairPayload payload;
        std::string error_message;
        if (!racshell::load_extract_pair("group",
                                         "group",
                                         left_groupid,
                                         right_groupid,
                                         debug,
                                         raw_json_output,
                                         payload,
                                         error_message))
        {
            throw std::runtime_error(error_message);
        }

        comparison.left.raw_response_json = std::move(payload.left_raw_response_json);
        comparison.right.raw_response_json = std::move(payload.right_raw_response_json);
        comparison.left.response_json = std::move(payload.left_response_json);
        comparison.right.response_json = std::move(payload.right_response_json);

        if (raw_json_output)
        {
            return comparison;
        }

        comparison.left.profile = std::move(payload.left_profile);
        comparison.left.base = std::move(payload.left_base);
        comparison.right.profile = std::move(payload.right_profile);
        comparison.right.base = std::move(payload.right_base);

        const nlohmann::json &left_profile = comparison.left.profile;
        const nlohmann::json &left_base = comparison.left.base;
        const nlohmann::json &right_profile = comparison.right.profile;
        const nlohmann::json &right_base = comparison.right.base;

        racshell::add_difference(comparison.differences, "owner",
                                 racshell::get_object_value(left_base, "base:owner"),
                                 racshell::get_object_value(right_base, "base:owner"));
        racshell::add_difference(comparison.differences, "created_date",
                                 racshell::get_object_value(left_base, "base:create_date"),
                                 racshell::get_object_value(right_base, "base:create_date"));
        racshell::add_difference(comparison.differences, "superior_group",
                                 racshell::get_object_value(left_base, "base:superior_group"),
                                 racshell::get_object_value(right_base, "base:superior_group"));
        racshell::add_difference(comparison.differences, "installation_data",
                                 racshell::get_object_value(left_base, "base:installation_data"),
                                 racshell::get_object_value(right_base, "base:installation_data"));
        racshell::add_difference(comparison.differences, "universal",
                                 racshell::get_object_value(left_base, "base:universal"),
                                 racshell::get_object_value(right_base, "base:universal"));
        racshell::add_difference(comparison.differences, "terminal_universal_access",
                                 racshell::get_object_value(left_base, "base:terminal_universal_access"),
                                 racshell::get_object_value(right_base, "base:terminal_universal_access"));

        if (compare_omvs)
        {
            racshell::add_difference(comparison.differences, "omvs",
                                     racshell::get_object_value(left_profile, "omvs"),
                                     racshell::get_object_value(right_profile, "omvs"));
        }

        if (compare_csdata)
        {
            racshell::add_difference(comparison.differences, "csdata",
                                     racshell::get_object_value(left_profile, "csdata"),
                                     racshell::get_object_value(right_profile, "csdata"));
        }

        if (compare_users)
        {
            add_connected_user_differences(left_base, right_base, comparison.differences);
        }

        comparison.identical = comparison.differences.empty();
        return comparison;
    }

    void add_connected_user_differences(const nlohmann::json &left_base,
                                        const nlohmann::json &right_base,
                                        nlohmann::json &differences)
    {
        const std::map<std::string, std::string> left_users = racshell::to_user_map(racshell::get_object_value(left_base, "base:connected_users"));
        const std::map<std::string, std::string> right_users = racshell::to_user_map(racshell::get_object_value(right_base, "base:connected_users"));

        nlohmann::json only_in_left = nlohmann::json::array();
        nlohmann::json only_in_right = nlohmann::json::array();
        nlohmann::json authority_mismatches = nlohmann::json::array();

        for (const auto &[userid, authority] : left_users)
        {
            const auto right_it = right_users.find(userid);
            if (right_it == right_users.end())
            {
                only_in_left.push_back({
                    {"userid", userid},
                    {"authority", authority}});
                continue;
            }

            if (authority != right_it->second)
            {
                authority_mismatches.push_back({
                    {"userid", userid},
                    {"left", authority},
                    {"right", right_it->second}});
            }
        }

        for (const auto &[userid, authority] : right_users)
        {
            if (left_users.find(userid) == left_users.end())
            {
                only_in_right.push_back({
                    {"userid", userid},
                    {"authority", authority}});
            }
        }

        if (!only_in_left.empty() || !only_in_right.empty() || !authority_mismatches.empty())
        {
            differences["connected_users"] = nlohmann::json::object();
            if (!only_in_left.empty())
            {
                differences["connected_users"]["only_in_left"] = only_in_left;
            }
            if (!only_in_right.empty())
            {
                differences["connected_users"]["only_in_right"] = only_in_right;
            }
            if (!authority_mismatches.empty())
            {
                differences["connected_users"]["authority_mismatches"] = authority_mismatches;
            }
        }
    }

} // namespace

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("comparegroups");

    program.add_argument("left-group")
        .help("first RACF group to compare");

    program.add_argument("right-group")
        .help("second RACF group to compare");

    racshell::add_toggle_argument(program, "-u", "--users", "compare connected users");
    racshell::add_toggle_argument(program, "-o", "--omvs", "compare OMVS segment");
    racshell::add_toggle_argument(program, "-x", "--csdata", "compare CSDATA segment");
    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON responses");

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

    const std::string left_groupid = program.get<std::string>("left-group");
    const std::string right_groupid = program.get<std::string>("right-group");
    if (left_groupid.length() > 8 || right_groupid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group name");
        return 1;
    }

    const bool compare_users = program.get<bool>("users");
    const bool compare_omvs = program.get<bool>("omvs");
    const bool compare_csdata = program.get<bool>("csdata");
    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");

    try
    {
        const GroupComparisonData comparison = build_group_comparison_data(left_groupid,
                                                                           right_groupid,
                                                                           compare_users,
                                                                           compare_omvs,
                                                                           compare_csdata,
                                                                           debug,
                                                                           all_json);

        if (json_output)
        {
            JsonFormatter formatter;
            std::cout << formatter.format(comparison);
            return 0;
        }

        TextFormatter formatter;
        std::cout << formatter.format(comparison);
        return 0;
    }
    catch (const std::exception &err)
    {
        racshell::print_error(std::cerr, err.what());
        return 1;
    }
}
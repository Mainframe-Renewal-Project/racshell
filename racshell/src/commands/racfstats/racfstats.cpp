#include <iostream>
#include <string>
#include <vector>

#include "include/racshell/command_helper.hpp"

namespace
{
    using json = nlohmann::json;

    bool request_sear(const json &request, bool debug, json &response, std::string &error)
    {
        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);
        if (result == nullptr || result->result_json == nullptr)
        {
            error = "No response returned by SEAR";
            return false;
        }
        const auto operation = racshell::validate_sear_operation_result(result->result_json);
        response = operation.response;
        error = operation.error_message;
        return operation.success;
    }

    bool extract_base(const char *admin_type, bool debug, json &base, std::string &error)
    {
        json response;
        if (!request_sear({{"operation", "extract"}, {"admin_type", admin_type}}, debug, response, error))
            return false;
        if (!response.contains("profile") || !response["profile"].is_object() ||
            !response["profile"].contains("base") || !response["profile"]["base"].is_object())
        {
            error = "missing profile data";
            return false;
        }
        base = response["profile"]["base"];
        return true;
    }

    bool count_profiles(const json &request, bool debug, json &count, std::string &error)
    {
        json response;
        if (!request_sear(request, debug, response, error))
        {
            if (!response.is_object() || !response.contains("return_codes") || !response["return_codes"].is_object())
                return false;
            const auto codes = racshell::extract_sear_return_codes(response);
            if (codes.sear_rc != 0 || codes.saf_rc != 4 || codes.racf_rc != 4 || codes.racf_reason != 4 ||
                !response.contains("profiles") || !response["profiles"].is_array() || !response["profiles"].empty())
                return false;
        }
        if (!response.contains("profiles") || !response["profiles"].is_array())
        {
            error = "missing profile search results";
            return false;
        }
        for (const auto &profile : response["profiles"])
        {
            if (!profile.is_string())
            {
                error = "invalid profile search results";
                return false;
            }
        }
        count = response["profiles"].size();
        return true;
    }
}

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("racfstats");
    program.add_description("Display a high-level RACF database and configuration overview.");
    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug SEAR requests and responses");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
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
    const bool debug = program.get<bool>("debug");
    json stats = json::object();
    json errors = json::object();
    std::string error;
    json options;
    if (!extract_base("racf-options", debug, options, error))
        errors["racf_options"] = error;

    struct Field
    {
        const char *name;
        const char *key;
        const char *label;
    };
    const std::vector<Field> fields = {
        {"max_incorrect_password_attempts", "base:max_incorrect_password_attempts", "Incorrect password/passphrase attempts allowed"},
        {"min_password_change_interval", "base:min_password_change_interval", "Minimum password change interval (days)"},
        {"max_password_change_interval", "base:max_password_change_interval", "Maximum password change interval (days)"},
        {"passphrase_change_interval", "base:passphrase_change_interval", "Passphrase change interval (days)"},
        {"password_expiration_warning", "base:password_expiration_warning", "Password expiration warning (days)"},
        {"password_history", "base:password_history", "Password history"},
        {"revoke_inactive_userids_interval", "base:revoke_inactive_userids_interval", "Inactive user revocation interval (days)"},
        {"mixed_case_password_support", "base:mixed_case_password_support", "Mixed-case password support"},
        {"special_character_password_support", "base:special_character_password_support", "Special-character password support"},
        {"password_encryption_algorithm", "base:password_encryption_algorithm", "Password encryption algorithm"}};
    for (const auto &field : fields)
        stats[field.name] = options.is_object() && options.contains(field.key) ? options[field.key] : json(nullptr);
    stats["log_operator_actions"] = options.is_object() && options.contains("base:log_operator_actions") ? options["base:log_operator_actions"] : json(nullptr);
    stats["log_commands_issued_by_special_users"] = options.is_object() && options.contains("base:log_commands_issued_by_special_users") ? options["base:log_commands_issued_by_special_users"] : json(nullptr);

    json prefix;
    json subsystem_userid;
    json number_of_defined_nodes;
    json rrsf;
    if (extract_base("racf-rrsf", debug, rrsf, error))
    {
        if (rrsf.contains("base:subsystem_operator_prefix"))
            prefix = rrsf["base:subsystem_operator_prefix"];
        if (rrsf.contains("base:subsystem_userid"))
            subsystem_userid = rrsf["base:subsystem_userid"];
        if (rrsf.contains("base:number_of_defined_nodes"))
            number_of_defined_nodes = rrsf["base:number_of_defined_nodes"];
    }
    else
        errors["rrsf"] = error;
    stats["rrsf_operator_prefix"] = prefix;
    stats["rrsf_subsystem_userid"] = subsystem_userid;
    stats["rrsf_number_of_defined_nodes"] = number_of_defined_nodes;

    json groups;
    if (!count_profiles({{"operation", "search"}, {"admin_type", "group"}}, debug, groups, error))
        errors["groups"] = error;
    stats["visible_groups"] = groups;

    json classes;
    if (!count_profiles({{"operation", "search"}, {"admin_type", "resource"}, {"class", racshell::to_racf_resource_class_text("CDT")}}, debug, classes, error))
        errors["cdt_profiles"] = error;
    stats["visible_cdt_profiles"] = classes;

    if (program.get<bool>("json"))
        std::cout << json({{"statistics", stats}, {"errors", errors}}).dump(2) << "\n";
    else
    {
        const bool color_enabled = racshell::color_output_enabled();
        racshell::print_colored_text(std::cout, racshell::terminal_color::cyan, "RACF OVERVIEW");
        std::cout << "\nFor all settings use commands such as rrsflist and racfctl list\n"
              << (color_enabled ? "\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81" : "------------") << "\n\n";
        const auto print_heading = [](std::string_view title, std::string_view color)
        {
            racshell::print_colored_text(std::cout, color, title) << "\n";
        };
        const auto print_value = [&stats, color_enabled](const char *name, const char *label)
        {
            const auto &value = stats[name];
            std::cout << (color_enabled ? "  \xe2\x94\x82 " : "  | ") << label << ": ";
            if (value.is_null())
                std::cout << "Unavailable";
            else if (value.is_string())
                std::cout << value.get<std::string>();
            else if (value.is_boolean())
                std::cout << (value.get<bool>() ? "Yes" : "No");
            else
                std::cout << value;
            std::cout << "\n";
        };
        print_heading("DATABASE", racshell::terminal_color::cyan);
        print_value("visible_groups", "Visible groups");
        print_value("visible_cdt_profiles", "Custom classes (CDT defined)");
        std::cout << "\n";
        print_heading("PASSWORDS & PASSPHRASES", racshell::terminal_color::cyan);
        for (const auto &field : fields)
            print_value(field.name, field.label);
        std::cout << "\n";
        print_heading("OPERATIONS", racshell::terminal_color::cyan);
        print_value("rrsf_operator_prefix", "RRSF operator command prefix");
        print_value("rrsf_subsystem_userid", "RRSF subsystem userid");
        print_value("rrsf_number_of_defined_nodes", "RRSF nodes defined");
        std::cout << "\n";
        print_heading("LOGGING", racshell::terminal_color::cyan);
        print_value("log_operator_actions", "Operator actions");
        print_value("log_commands_issued_by_special_users", "Commands issued by SPECIAL users");
        for (const auto &entry : errors.items())
            racshell::print_error(std::cerr, entry.key() + ": " + entry.value().get<std::string>());
    }
    return errors.empty() ? 0 : 1;
}
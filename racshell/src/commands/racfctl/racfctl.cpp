#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "include/racshell/command_helper.hpp"
#include "include/racshell/output_formatter.hpp"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("racfctl");
    program.add_description("Display or alter RACF system options (SETROPTS).");
    argparse::ArgumentParser list("list");
    list.add_description("Display RACF system options with optional class lists.");
    racshell::add_toggle_argument(list, "-c", "--classes", "display all class lists");
    struct ClassListOption
    {
        const char *flag;
        const char *key;
        const char *help;
    };
    const std::vector<ClassListOption> class_list_options = {
        {"--active-classes", "base:active_classes", "display active classes"},
        {"--raclist", "base:raclist", "display RACLIST classes"},
        {"--generic", "base:generic_profile_checking_classes", "display generic profile checking classes"},
        {"--genlist", "base:generic_profile_sharing_classes", "display generic profile sharing classes"},
        {"--generic-command-classes", "base:generic_command_classes", "display generic command classes"},
        {"--audit-classes", "base:audit_classes", "display profile change auditing classes"},
        {"--statistics-classes", "base:statistics_classes", "display statistics classes"},
        {"--global-access-classes", "base:global_access_classes", "display global access classes"},
    };
    for (const auto &option : class_list_options)
        list.add_argument(option.flag).help(option.help).flag();
    list.add_argument("--logging-classes").help("display audit logging class lists").flag();
    argparse::ArgumentParser modify("modify");
    modify.add_description("Alter RACF system options.");
    modify.add_argument("traits")
        .help("additional system options to alter as key=value pairs")
        .nargs(argparse::nargs_pattern::any);
    argparse::ArgumentParser refresh("refresh");
    refresh.add_description("Refresh in-storage profiles for a RACF class.");
    refresh.add_argument("class")
        .help("RACF class to refresh");
    racshell::add_toggle_argument(refresh, "-g", "--generic", "refresh GENERIC profiles instead of RACLIST profiles");

    enum class TraitType { text, integer, boolean };
    struct TraitOption
    {
        const char *flag;
        const char *key;
        const char *help;
        TraitType type = TraitType::text;
    };
    const std::vector<TraitOption> trait_options = {
        {"--classact", "add:base:active_classes", "activate classes (quote space-separated class names)"},
        {"--noclassact", "remove:base:active_classes", "deactivate classes"},
        {"--raclist", "add:base:raclist", "enable RACLIST processing for classes"},
        {"--noraclist", "remove:base:raclist", "disable RACLIST processing for classes"},
        {"--generic", "base:generic_profile_checking_classes", "enable generic profile checking for classes"},
        {"--genlist", "add:base:generic_profile_sharing_classes", "enable generic profile sharing for classes"},
        {"--nogenlist", "remove:base:generic_profile_sharing_classes", "disable generic profile sharing for classes"},
        {"--audit", "add:base:audit_classes", "audit profile changes for classes"},
        {"--noaudit", "remove:base:audit_classes", "stop auditing profile changes for classes"},
        {"--password-interval", "base:max_password_change_interval", "maximum password change interval"},
        {"--min-password-interval", "base:min_password_change_interval", "minimum password change interval"},
        {"--passphrase-interval", "base:passphrase_change_interval", "passphrase change interval"},
        {"--password-warning", "base:password_expiration_warning", "password expiration warning interval"},
        {"--inactive-interval", "base:revoke_inactive_userids_interval", "inactive userid revocation interval"},
        {"--password-history", "base:password_history", "number of previous passwords to remember", TraitType::integer},
        {"--max-password-attempts", "base:max_incorrect_password_attempts", "incorrect password attempts before revocation", TraitType::integer},
        {"--mixed-case-passwords", "base:mixed_case_password_support", "enable or disable mixed-case passwords (true/false)", TraitType::boolean},
        {"--special-character-passwords", "base:special_character_password_support", "enable or disable special password characters (true/false)", TraitType::boolean},
        {"--program-control", "base:program_control", "enable or disable program control (true/false)", TraitType::boolean},
        {"--special-user-audit", "base:log_commands_issued_by_special_users", "audit SPECIAL user commands (true/false)", TraitType::boolean},
        {"--operator-audit", "base:log_operator_actions", "audit operator actions (true/false)", TraitType::boolean},
        {"--group-list", "base:list_of_groups_access_checking", "enable list-of-groups access checking (true/false)", TraitType::boolean},
        {"--initstat", "base:record_user_verification_statistics", "record user verification statistics (true/false)", TraitType::boolean},
    };
    for (const auto &option : trait_options)
    {
        auto &argument = modify.add_argument(option.flag).help(option.help);
        if (option.type == TraitType::integer)
        {
            argument.scan<'u', unsigned int>();
        }
        else if (option.type == TraitType::boolean)
        {
            argument.choices("true", "false");
        }
    }
    for (auto *command : {&list, &modify, &refresh})
    {
        racshell::add_no_color_argument(*command);
        racshell::add_toggle_argument(*command, "-d", "--debug", "debug sear request and response");
        racshell::add_toggle_argument(*command, "-j", "--json", "output as JSON");
        racshell::add_toggle_argument(*command, "-a", "--all-json", "output full raw SEAR JSON response");
        program.add_subparser(*command);
    }

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

    if (!program.is_subcommand_used("list") && !program.is_subcommand_used("modify") && !program.is_subcommand_used("refresh"))
    {
        racshell::print_error(std::cerr, "Expected a subcommand: list, modify or refresh");
        std::cerr << program;
        return 1;
    }

    const bool modifying = program.is_subcommand_used("modify");
    const bool refreshing = program.is_subcommand_used("refresh");
    const bool alter = modifying || refreshing;
    auto &command = refreshing ? refresh : (modifying ? modify : list);
    racshell::set_color_output_enabled(!command.get<bool>("no-color"));
    nlohmann::json request = {
        {"operation", alter ? "alter" : "extract"},
        {"admin_type", "racf-options"}};

    if (modifying)
    {
        nlohmann::json traits;
        if (!racshell::parse_traits(command.get<std::vector<std::string>>("traits"), traits))
        {
            return 1;
        }
        for (const auto &option : trait_options)
        {
            if (!command.is_used(option.flag))
            {
                continue;
            }
            const std::string key = option.key;
            if (traits.contains(key) || traits.contains("set:" + key))
            {
                racshell::print_error(std::cerr, "trait supplied both positionally and through " + std::string(option.flag));
                return 1;
            }
            if (option.type == TraitType::integer)
            {
                traits[key] = command.get<unsigned int>(option.flag);
            }
            else if (option.type == TraitType::boolean)
            {
                traits[key] = command.get<std::string>(option.flag) == "true";
            }
            else
            {
                traits[key] = command.get<std::string>(option.flag);
            }
        }
        if (traits.empty())
        {
            racshell::print_error(std::cerr, "modify requires at least one trait or system option flag");
            return 1;
        }
        for (const auto &trait : traits.items())
        {
            if (trait.key().empty())
            {
                racshell::print_error(std::cerr, "trait name must not be empty");
                return 1;
            }
        }
        request["traits"] = traits;
    }

    if (refreshing)
    {
        std::string class_name = racshell::to_racf_identifier_text(command.get<std::string>("class"));
        if (class_name.empty() || class_name.length() > 8 ||
            !std::all_of(class_name.begin(), class_name.end(), [](unsigned char character)
            {
                return std::isalnum(character) || character == '@' || character == '#' || character == '$';
            }))
        {
            racshell::print_error(std::cerr, "Invalid RACF class: expected 1-8 letters, digits, @, # or $");
            return 1;
        }
        std::transform(class_name.begin(), class_name.end(), class_name.begin(), [](unsigned char character)
        {
            return static_cast<char>(std::toupper(character));
        });
        request["traits"] = {
            {command.get<bool>("generic") ? "base:generic_profile_checking_classes" : "add:base:raclist", class_name},
            {"base:refresh", true}};
    }

    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), command.get<bool>("debug"));
    if (result == nullptr || result->result_json == nullptr)
    {
        racshell::print_error(std::cerr, "No response returned by SEAR");
        return 1;
    }

    const racshell::SearOperationInfo operation = racshell::validate_sear_operation_result(result->result_json);
    const bool all_json = command.get<bool>("all-json");
    if (all_json)
    {
        std::cout << result->result_json << "\n";
    }
    if (!operation.success)
    {
        racshell::print_error(std::cerr, operation.error_message);
        racshell::print_sear_errors(operation.response, std::cerr);
        return 1;
    }

    if (alter)
    {
        if (!all_json)
        {
            if (command.get<bool>("json"))
            {
                const nlohmann::json output = {{"admin_type", "racf-options"}, {"status", refreshing ? "refreshed" : "altered"}};
                std::cout << output.dump(2) << "\n";
            }
            else
            {
                racshell::print_operation_success(std::cout, "alter", refreshing ? "RACF profiles refreshed successfully." : "RACF system options altered successfully.");
            }
        }
        return 0;
    }

    const racshell::SearResponseInfo options = racshell::validate_sear_response(result->result_json, "RACF system options");
    if (!options.success)
    {
        racshell::print_error(std::cerr, options.error_message);
        return 1;
    }
    if (all_json)
    {
        return 0;
    }
    RacfOptionsData data{options.profile};
    if (!command.get<bool>("classes"))
    {
        for (auto &segment : data.profile.items())
        {
            auto &fields = segment.value();
            for (auto field = fields.begin(); field != fields.end();)
            {
                const auto &key = field.key();
                const bool class_list = key.find("_classes") != std::string::npos || key == "base:raclist";
                bool selected = command.get<bool>("logging-classes") && key.find("base:audit_log_") == 0;
                for (const auto &option : class_list_options)
                    selected = selected || (key == option.key && command.get<bool>(option.flag));
                if (class_list && !selected)
                    field = fields.erase(field);
                else
                    ++field;
            }
        }
    }
    if (command.get<bool>("json"))
    {
        std::cout << JsonFormatter().format(data);
    }
    else
    {
        std::cout << TextFormatter().format(data);
    }
    std::cout << "\n";
    return 0;
}
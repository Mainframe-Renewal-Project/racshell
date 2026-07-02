#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>

#include "sear/sear.h"

namespace racshell
{

    namespace terminal_color
    {

        /**
         * @brief ANSI reset sequence.
         */
        inline constexpr std::string_view reset = "\x1b[0m";

        /**
         * @brief ANSI red foreground sequence.
         */
        inline constexpr std::string_view red = "\x1b[31m";

        /**
         * @brief ANSI green foreground sequence.
         */
        inline constexpr std::string_view green = "\x1b[32m";

        /**
         * @brief ANSI yellow foreground sequence.
         */
        inline constexpr std::string_view yellow = "\x1b[33m";

        /**
         * @brief ANSI blue foreground sequence.
         */
        inline constexpr std::string_view blue = "\x1b[34m";

    } // namespace terminal_color

    /**
     * @brief Accesses the process-wide color output toggle.
     * @return Reference to the color-enabled state.
     *
     * The variable is initialized to true once and remains allocated until process exit; its value may be changed during execution by calling set_color_output_enabled(bool enabled).
     */
    inline bool &color_output_enabled()
    {
        static bool enabled = true;
        return enabled;
    }

    /**
     * @brief Enables or disables ANSI color output globally.
     * @param enabled True to emit color sequences, false to suppress them.
     */
    inline void set_color_output_enabled(bool enabled)
    {
        color_output_enabled() = enabled;
    }

    /**
     * @brief Writes a color sequence to the output stream when colors are enabled.
     * @param output Stream to write to.
     * @param color ANSI color escape sequence.
     * @return The same output stream for chaining.
     */
    inline std::ostream &colorize(std::ostream &output, std::string_view color)
    {
        if (!color_output_enabled())
        {
            return output;
        }
        return output << color;
    }

    /**
     * @brief Writes the ANSI reset sequence when colors are enabled.
     * @param output Stream to write to.
     * @return The same output stream for chaining.
     */
    inline std::ostream &reset_color(std::ostream &output)
    {
        if (!color_output_enabled())
        {
            return output;
        }
        return output << terminal_color::reset;
    }

    /**
     * @brief Writes text in the given color and automatically resets the color.
     * @param output Stream to write to.
     * @param color ANSI color escape sequence.
     * @param text Text to print using the given color.
     * @return The same output stream for chaining.
     */
    inline std::ostream &print_colored_text(std::ostream &output,
                                            std::string_view color,
                                            std::string_view text)
    {
        colorize(output, color) << text;
        return reset_color(output);
    }

    /**
     * @brief Prints a standardized error prefix.
     * @param output Stream to write to.
     *
     * Output format: "RACSHELL Error: " where "RACSHELL Error" is red when enabled; everything after (colon, space, and any subsequent text) is default color.
     */
    inline void print_error_prefix(std::ostream &output)
    {
        colorize(output, terminal_color::red) << "RACSHELL Error";
        reset_color(output) << ": ";
    }

    /**
     * @brief Prints a full standardized error line.
     * @param output Stream to write to.
     * @param message Error message text without prefix.
     */
    inline void print_error(std::ostream &output, const std::string &message)
    {
        print_error_prefix(output);
        output << message << "\n";
    }

    /**
     * @brief Prints a standardized success prefix.
     * @param output Stream to write to.
     *
     * Output format: "RACSHELL Success: " where "RACSHELL Success" is green when enabled; everything after (colon, space, and any subsequent text) is default color.
     */
    inline void print_success_prefix(std::ostream &output)
    {
        colorize(output, terminal_color::green) << "RACSHELL Success";
        reset_color(output) << ": ";
    }

    /**
     * @brief Adds a conventional boolean toggle argument to an argparse program.
     * @param program Target argument parser.
     * @param short_name Short flag form, e.g. "-d".
     * @param long_name Long flag form, e.g. "--debug".
     * @param help_text Help text for the argument.
     */
    inline void add_toggle_argument(argparse::ArgumentParser &program,
                                    const char *short_name,
                                    const char *long_name,
                                    const char *help_text)
    {
        program.add_argument(short_name, long_name)
            .help(help_text)
            .default_value(false)
            .implicit_value(true)
            .nargs(0);
    }

    /**
     * @brief Adds the -n/--no-color flag to a command.
     * @param program Target argument parser.
     */
    inline void add_no_color_argument(argparse::ArgumentParser &program)
    {
        add_toggle_argument(program, "-n", "--no-color", "disable colored output");
    }

    /**
     * @brief Parsed response details for extract-style SEAR operations.
     */
    struct SearResponseInfo
    {
        bool success;
        std::string error_message;
        nlohmann::json profile;
        nlohmann::json base;
    };

    /**
     * @brief Parsed response details for add/alter/delete-style SEAR operations.
     */
    struct SearOperationInfo
    {
        bool success;
        std::string error_message;
        nlohmann::json response;
    };

    /**
     * @brief Canonical SEAR/RACF return code bundle.
     */
    struct SearReturnCodes
    {
        int sear_rc = 0;
        int saf_rc = 0;
        int racf_rc = 0;
        int racf_reason = 0;
    };

    /**
     * @brief Parses a SEAR response JSON document.
     * @param result_json Raw response string.
     * @param response Parsed JSON output on success.
     * @param error_message Populated parse error text on failure.
     * @return True when JSON parsing succeeds.
     */
    inline bool parse_sear_response_json(const char *result_json,
                                         nlohmann::json &response,
                                         std::string &error_message)
    {
        try
        {
            response = nlohmann::json::parse(result_json);
            return true;
        }
        catch (const std::exception &e)
        {
            error_message = std::string("Failed to parse SEAR response: ") + e.what();
            return false;
        }
    }

    /**
     * @brief Extracts return-code fields from a SEAR response.
     * @param response Parsed response object.
     * @return Return-code bundle with missing values defaulted to zero.
     */
    inline SearReturnCodes extract_sear_return_codes(const nlohmann::json &response)
    {
        const nlohmann::json return_codes = response.value("return_codes", nlohmann::json::object());
        return {
            return_codes.value("sear_return_code", 0),
            return_codes.value("saf_return_code", 0),
            return_codes.value("racf_return_code", 0),
            return_codes.value("racf_reason_code", 0)};
    }

    /**
     * @brief Validates that the SEAR/RACF return codes indicate success.
     * @param codes Return-code bundle.
     * @param error_message Populated failure details when any code indicates error.
     * @return True when the operation is successful.
     */
    inline bool validate_sear_return_codes(const SearReturnCodes &codes, std::string &error_message)
    {
        if (codes.sear_rc == 0 && codes.saf_rc == 0 && codes.racf_rc == 0)
        {
            return true;
        }

        error_message = "request failed (sear=" + std::to_string(codes.sear_rc) + ", saf=" + std::to_string(codes.saf_rc) + ", racf=" + std::to_string(codes.racf_rc) + ", reason=" + std::to_string(codes.racf_reason) + ")";
        return false;
    }

    /**
     * @brief Validates an operation-style SEAR response.
     * @param result_json Raw SEAR response JSON string.
     * @return Populated operation info including success state and parsed response.
     */
    inline SearOperationInfo validate_sear_operation_result(const char *result_json)
    {
        SearOperationInfo info;
        info.success = false;
        info.response = nlohmann::json::object();

        if (!parse_sear_response_json(result_json, info.response, info.error_message))
        {
            return info;
        }

        const SearReturnCodes codes = extract_sear_return_codes(info.response);
        if (!validate_sear_return_codes(codes, info.error_message))
        {
            return info;
        }

        info.success = true;
        return info;
    }

    /**
     * @brief Prints each string entry from the SEAR "errors" array.
     * @param response Parsed SEAR response object.
     * @param output Stream to write to.
     */
    inline void print_sear_errors(const nlohmann::json &response, std::ostream &output)
    {
        if (response.contains("errors") && response["errors"].is_array())
        {
            for (const auto &err : response["errors"])
            {
                if (err.is_string())
                {
                    output << "  " << err.get<std::string>() << "\n";
                }
            }
        }
    }

    /**
     * @brief Validates and extracts profile/base from an extract-style SEAR response.
     * @param result_json Raw SEAR response JSON string.
     * @param entity_type Logical entity name used in error messages.
     * @return Populated extract response info with success state and payload objects.
     */
    inline SearResponseInfo validate_sear_response(const char *result_json, const std::string &entity_type)
    {
        SearResponseInfo info;
        info.success = false;

        nlohmann::json response;
        if (!parse_sear_response_json(result_json, response, info.error_message))
        {
            return info;
        }

        const SearReturnCodes codes = extract_sear_return_codes(response);
        if (!validate_sear_return_codes(codes, info.error_message))
        {
            return info;
        }

        // Check profile and base structure
        const auto profile_it = response.find("profile");
        if (profile_it == response.end() || !profile_it->is_object())
        {
            info.error_message = entity_type + " not found or missing profile data";
            return info;
        }

        const auto base_it = profile_it->find("base");
        if (base_it == profile_it->end() || !base_it->is_object())
        {
            info.error_message = entity_type + " not found or missing profile data";
            return info;
        }

        info.success = true;
        info.profile = *profile_it;
        info.base = *base_it;
        return info;
    }

    /**
     * @brief Parses traits arguments into a JSON object.
     * @param trait_args Input list of values in key=value format.
     * @param traits Output JSON object with parsed values.
     * @return True on success; false when an input item is malformed.
     *
     * Value coercion order:
     * 1. case-insensitive booleans (true/false)
     * 2. integers via std::stoll
     * 3. fallback to string
     */
    inline bool parse_traits(const std::vector<std::string> &trait_args, nlohmann::json &traits)
    {
        traits = nlohmann::json::object();
        for (const auto &trait : trait_args)
        {
            auto sep = trait.find('=');
            if (sep == std::string::npos)
            {
                print_error(std::cerr, "trait must be in key=value format, got: " + trait);
                return false;
            }
            std::string key = trait.substr(0, sep);
            std::string value = trait.substr(sep + 1);

            // Convert to lowercase for case-insensitive comparison
            std::string value_lower = value;
            std::transform(value_lower.begin(), value_lower.end(), value_lower.begin(),
                           [](unsigned char c)
                           { return std::tolower(c); });

            if (value_lower == "true")
            {
                traits[key] = true;
                continue;
            }
            if (value_lower == "false")
            {
                traits[key] = false;
                continue;
            }

            try
            {
                size_t pos;
                long long int_val = std::stoll(value, &pos);
                if (pos == value.size())
                {
                    traits[key] = int_val;
                    continue;
                }
            }
            catch (...)
            {
            }

            traits[key] = value;
        }
        return true;
    }

    /**
     * @brief Safely extracts a string property from a JSON object.
     * @param obj Source JSON object.
     * @param key Property name.
     * @param out Destination variable updated when the key exists and is a string.
     */
    inline void assign_string(const nlohmann::json &obj, const char *key, std::string &out)
    {
        auto element = obj.find(key);
        if (element != obj.end() && element->is_string())
        {
            out = element->get<std::string>();
        }
    }

    /**
     * @brief Safely extracts a boolean property from a JSON object.
     * @param obj Source JSON object.
     * @param key Property name.
     * @param out Destination variable updated when the key exists and is a boolean.
     */
    inline void assign_bool(const nlohmann::json &obj, const char *key, bool &out)
    {
        auto element = obj.find(key);
        if (element != obj.end() && element->is_boolean())
        {
            out = element->get<bool>();
        }
    }

    /**
     * @brief Describes a single CLI flag that maps directly to a SEAR trait.
     */
    struct TraitArg
    {
        std::string short_flag;
        std::string long_flag;
        std::string trait_key;
        std::string help;
        bool is_flag = false;
    };

    /**
     * @brief Registers a list of trait arguments with an argparse program.
     * @param program Target argument parser.
     * @param trait_args List of trait argument descriptors.
     */
    inline void register_trait_args(argparse::ArgumentParser &program,
                                    const std::vector<TraitArg> &trait_args)
    {
        for (const auto &t : trait_args)
        {
            auto &arg = t.short_flag.empty()
                ? program.add_argument(t.long_flag)
                : program.add_argument(t.short_flag, t.long_flag);
            arg.help(t.help);
            if (t.is_flag)
                arg.flag();
        }
    }

    /**
     * @brief Applies parsed trait argument values into a SEAR request JSON.
     * @param program Parsed argument parser.
     * @param request SEAR request JSON object to populate.
     * @param trait_args List of trait argument descriptors.
     */
    inline void apply_trait_args(argparse::ArgumentParser &program,
                                 nlohmann::json &request,
                                 const std::vector<TraitArg> &trait_args)
    {
        for (const auto &t : trait_args)
        {
            if (t.is_flag)
            {
                if (program.get<bool>(t.long_flag))
                    request["traits"][t.trait_key] = true;
            }
            else if (auto v = program.present<std::string>(t.long_flag))
            {
                request["traits"][t.trait_key] = *v;
            }
        }
    }

    /**
     * @brief Converts a JSON value into stable human-readable text.
     * @param value JSON value to format.
     * @return "<missing>" for null, plain string for JSON strings, dump() for other types.
     */
    inline std::string value_to_text(const nlohmann::json &value)
    {
        if (value.is_null())
        {
            return "<missing>";
        }
        if (value.is_string())
        {
            return value.get<std::string>();
        }
        return value.dump();
    }

    /**
     * @brief Converts a connected_users JSON array into a userid-to-authority map.
     * @param users JSON array from a group extract response.
     * @return Map of userid to authority, ignoring malformed entries.
     */
    inline std::map<std::string, std::string> to_user_map(const nlohmann::json &users)
    {
        std::map<std::string, std::string> result;
        if (!users.is_array())
        {
            return result;
        }

        for (const auto &user : users)
        {
            if (!user.is_object())
            {
                continue;
            }

            const std::string userid = user.contains("base:connected_userid")
                                           ? value_to_text(user["base:connected_userid"])
                                           : (user.contains("userid") ? value_to_text(user["userid"]) : std::string());
            if (userid.empty() || userid == "<missing>")
            {
                continue;
            }

            const nlohmann::json authority_value = user.contains("base:connected_user_authority")
                                                       ? user["base:connected_user_authority"]
                                                       : (user.contains("authority") ? user["authority"] : nlohmann::json());
            result[userid] = value_to_text(authority_value);
        }

        return result;
    }

    /**
     * @brief Executes a generic extract request and returns the raw SEAR JSON response.
     * @param admin_type RACF admin type, e.g. "user" or "group".
     * @param id_key Request key used for the entity id, e.g. "userid".
     * @param entity_id Entity identifier to extract.
     * @param debug Enables SEAR debug mode.
     * @return Raw SEAR response JSON string or empty string when no response is returned.
     */
    inline std::string execute_extract_request(const char *admin_type,
                                               const char *id_key,
                                               const std::string &entity_id,
                                               bool debug)
    {
        const nlohmann::json request = {
            {"operation", "extract"},
            {"admin_type", admin_type},
            {id_key, entity_id}};

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);
        if (result == nullptr || result->result_json == nullptr)
        {
            return {};
        }

        return result->result_json;
    }

    /**
     * @brief Validates and extracts profile/base from a generic extract response.
     * @param result_json Raw SEAR response JSON string.
     * @param entity_type Logical entity name used in errors, e.g. "user".
     * @param entity_id Entity identifier used in errors.
     * @param profile Parsed profile JSON output.
     * @param base Parsed base JSON output.
     * @param error_message Populated failure details on error.
     * @return True when extraction data is valid and available.
     */
    inline bool parse_extract_payload(const std::string &result_json,
                                      const std::string &entity_type,
                                      const std::string &entity_id,
                                      nlohmann::json &profile,
                                      nlohmann::json &base,
                                      std::string &error_message)
    {
        if (result_json.empty())
        {
            error_message = "No response returned for " + entity_type + " " + entity_id;
            return false;
        }

        SearResponseInfo info = validate_sear_response(result_json.c_str(), entity_type);
        if (!info.success)
        {
            error_message = info.error_message;
            return false;
        }

        profile = std::move(info.profile);
        base = std::move(info.base);
        return true;
    }

    /**
     * @brief Container for paired extract request payloads.
     */
    struct ExtractPairPayload
    {
        std::string left_raw_response_json;
        std::string right_raw_response_json;
        nlohmann::json left_response_json;
        nlohmann::json right_response_json;
        nlohmann::json left_profile;
        nlohmann::json left_base;
        nlohmann::json right_profile;
        nlohmann::json right_base;
    };

    /**
     * @brief Loads and validates extract payloads for two entities.
     * @param admin_type RACF admin type, e.g. "user" or "group".
     * @param id_key Request key used for the entity id, e.g. "userid".
     * @param left_id Left-side entity identifier.
     * @param right_id Right-side entity identifier.
     * @param debug Enables SEAR debug mode.
     * @param raw_json_output When true, only raw/parsed response JSON is populated.
     * @param payload Output payload container.
     * @param error_message Populated failure details on error.
     * @return True when payloads are loaded successfully.
     */
    inline bool load_extract_pair(const char *admin_type,
                                  const char *id_key,
                                  const std::string &left_id,
                                  const std::string &right_id,
                                  bool debug,
                                  bool raw_json_output,
                                  ExtractPairPayload &payload,
                                  std::string &error_message)
    {
        payload.left_raw_response_json = execute_extract_request(admin_type, id_key, left_id, debug);
        payload.right_raw_response_json = execute_extract_request(admin_type, id_key, right_id, debug);

        if (raw_json_output)
        {
            std::string parse_error;
            parse_sear_response_json(payload.left_raw_response_json.c_str(), payload.left_response_json, parse_error);
            parse_sear_response_json(payload.right_raw_response_json.c_str(), payload.right_response_json, parse_error);
            return true;
        }

        if (!parse_extract_payload(payload.left_raw_response_json,
                                   admin_type,
                                   left_id,
                                   payload.left_profile,
                                   payload.left_base,
                                   error_message))
        {
            error_message = left_id + ": " + error_message;
            return false;
        }

        if (!parse_extract_payload(payload.right_raw_response_json,
                                   admin_type,
                                   right_id,
                                   payload.right_profile,
                                   payload.right_base,
                                   error_message))
        {
            error_message = right_id + ": " + error_message;
            return false;
        }

        return true;
    }

    /**
     * @brief Returns an object value when present; otherwise returns JSON null.
     * @param object Source JSON object.
     * @param key Key to fetch.
     * @return Value for key or null when key is absent.
     */
    inline nlohmann::json get_object_value(const nlohmann::json &object, const char *key)
    {
        const auto it = object.find(key);
        if (it == object.end())
        {
            return nullptr;
        }
        return *it;
    }

    /**
     * @brief Adds a left/right difference object when two values differ.
     * @param differences Destination differences JSON object.
     * @param label Difference label.
     * @param left Left-side value.
     * @param right Right-side value.
     */
    inline void add_difference(nlohmann::json &differences,
                               const char *label,
                               const nlohmann::json &left,
                               const nlohmann::json &right)
    {
        if (left != right)
        {
            differences[label] = {
                {"left", left},
                {"right", right}};
        }
    }

    /**
     * @brief Adds set-based list differences under only_in_left/only_in_right.
     * @param differences Destination differences JSON object.
     * @param label Difference label.
     * @param left Left-side list.
     * @param right Right-side list.
     */
    inline void add_list_difference(nlohmann::json &differences,
                                    const char *label,
                                    const std::vector<std::string> &left,
                                    const std::vector<std::string> &right)
    {
        if (left == right)
        {
            return;
        }

        const std::set<std::string> left_set(left.begin(), left.end());
        const std::set<std::string> right_set(right.begin(), right.end());

        nlohmann::json only_in_left = nlohmann::json::array();
        nlohmann::json only_in_right = nlohmann::json::array();

        for (const auto &entry : left_set)
        {
            if (right_set.find(entry) == right_set.end())
            {
                only_in_left.push_back(entry);
            }
        }

        for (const auto &entry : right_set)
        {
            if (left_set.find(entry) == left_set.end())
            {
                only_in_right.push_back(entry);
            }
        }

        differences[label] = {
            {"only_in_left", only_in_left},
            {"only_in_right", only_in_right}};
    }

} // namespace racshell

#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"

namespace racshell
{

    /**
     * @brief Configuration for a generic profile search command.
     */
    struct SearchCommandSpec
    {
        const char *command_name;
        const char *admin_type;
        const char *filter_key;
        const char *filter_help;
        const char *filter_validation_error;
        std::size_t filter_max_length;
        const char *empty_results_message;
        const char *results_label;
        std::function<void(argparse::ArgumentParser &)> setup_extra_args = nullptr;
        std::function<bool(const argparse::ArgumentParser &, std::string &)> validate_extra_args = nullptr;
        std::function<void(const argparse::ArgumentParser &, nlohmann::json &)> apply_extra_args = nullptr;
        std::function<std::string(const argparse::ArgumentParser &, const std::string &)> build_text_header = nullptr;
    };

    /**
     * @brief Runs a generic search command that returns profile names from SEAR.
     * @param argc Argument count.
     * @param argv Argument values.
     * @param spec Search command behavior and labels.
     * @return Process exit code.
     */
    inline int run_search_command(int argc, char *argv[], const SearchCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        program.add_argument("-f", "--filter")
            .help(spec.filter_help)
            .default_value(std::string(""));

        if (spec.setup_extra_args)
        {
            spec.setup_extra_args(program);
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

        const std::string display_filter = program.get<std::string>("filter");
        if (display_filter.length() > spec.filter_max_length)
        {
            print_error(std::cerr, spec.filter_validation_error);
            return 1;
        }

        const std::string request_filter = to_racf_identifier_text(display_filter);

        std::string extra_error;
        if (spec.validate_extra_args && !spec.validate_extra_args(program, extra_error))
        {
            print_error(std::cerr, extra_error);
            return 1;
        }

        const bool debug = program.get<bool>("debug");
        const bool json_output = program.get<bool>("json");
        const bool all_json = program.get<bool>("all-json");

        nlohmann::json request = {
            {"operation", "search"},
            {"admin_type", spec.admin_type}};

        if (!request_filter.empty())
        {
            request[spec.filter_key] = request_filter;
        }

        if (spec.apply_extra_args)
        {
            spec.apply_extra_args(program, request);
        }

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        nlohmann::json response;
        std::string error_message;
        if (!parse_sear_response_json(result->result_json, response, error_message))
        {
            print_error(std::cerr, error_message);
            return 1;
        }

        const SearReturnCodes codes = extract_sear_return_codes(response);
        if (!validate_sear_return_codes(codes, error_message))
        {
            print_error(std::cerr, error_message);
            return 1;
        }

        std::vector<std::string> profiles;
        if (response.contains("profiles") && response["profiles"].is_array())
        {
            for (const auto &profile : response["profiles"])
            {
                if (profile.is_string())
                {
                    profiles.push_back(from_racf_identifier_text(profile.get<std::string>()));
                }
            }
        }

        if (json_output)
        {
            nlohmann::json output = nlohmann::json::array();
            for (const auto &profile : profiles)
            {
                output.push_back(profile);
            }
            std::cout << output.dump(2) << "\n";
            return 0;
        }

        if (profiles.empty())
        {
            std::cout << spec.empty_results_message << "\n";
            return 0;
        }

        if (spec.build_text_header)
        {
            std::cout << spec.build_text_header(program, display_filter) << "\n";
        }
        else
        {
            std::cout << spec.results_label;
            if (!display_filter.empty())
            {
                std::cout << " matching '" << display_filter << "'";
            }
            std::cout << ":\n";
        }

        for (const auto &profile : profiles)
        {
            std::cout << "  " << profile << "\n";
        }

        return 0;
    }

} // namespace racshell

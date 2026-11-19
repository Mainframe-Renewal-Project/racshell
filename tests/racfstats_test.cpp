#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define main racfstats_main
#include "../racshell/src/commands/racfstats/racfstats.cpp"
#undef main

namespace
{
    std::vector<nlohmann::json> requests;
    std::vector<nlohmann::json> responses;
    std::string result_text;
    sear_result_t result = {};

    int run_command(const std::vector<const char *> &arguments, std::string &output)
    {
        std::vector<char *> argv;
        for (const char *argument : arguments)
            argv.push_back(const_cast<char *>(argument));
        std::ostringstream stream;
        auto *previous = std::cout.rdbuf(stream.rdbuf());
        const int status = racfstats_main(static_cast<int>(argv.size()), argv.data());
        std::cout.rdbuf(previous);
        output = stream.str();
        return status;
    }
}

extern "C" sear_result_t *sear(const char *request_json, int length, bool)
{
    requests.push_back(nlohmann::json::parse(std::string(request_json, length)));
    result_text = responses.at(requests.size() - 1).dump();
    result.result_json = &result_text[0];
    return &result;
}

int main()
{
    const nlohmann::json success = {{"sear_return_code", 0}, {"saf_return_code", 0}, {"racf_return_code", 0}, {"racf_reason_code", 0}};
    const nlohmann::json empty = {{"sear_return_code", 0}, {"saf_return_code", 4}, {"racf_return_code", 4}, {"racf_reason_code", 4}};
    responses = {
        {{"return_codes", success}, {"profile", {{"base", {{"base:max_password_change_interval", 90}, {"base:max_incorrect_password_attempts", 5}, {"base:log_operator_actions", false}, {"base:log_commands_issued_by_special_users", true}, {"base:special_character_password_support", true}}}}}},
        {{"return_codes", success}, {"profile", {{"base", {{"base:subsystem_operator_prefix", "RACF"}}}}}},
        {{"return_codes", success}, {"profiles", {"GROUP1", "GROUP2"}}},
        {{"return_codes", empty}, {"profiles", nlohmann::json::array()}}};
    std::string output;
    assert(run_command({"racfstats", "--json"}, output) == 0);
    const auto parsed = nlohmann::json::parse(output);
    assert(parsed["statistics"]["visible_groups"] == 2);
    assert(parsed["statistics"]["visible_cdt_profiles"] == 0);
    assert(parsed["statistics"]["max_password_change_interval"] == 90);
    assert(parsed["statistics"]["max_incorrect_password_attempts"] == 5);
    assert(parsed["statistics"]["special_character_password_support"] == true);
    assert(parsed["statistics"]["log_operator_actions"] == false);
    assert(parsed["statistics"]["log_commands_issued_by_special_users"] == true);
    assert(parsed["statistics"]["passphrase_change_interval"].is_null());
    assert(parsed["statistics"]["rrsf_operator_prefix"] == "RACF");
    assert(parsed["errors"].empty());
    assert(requests.size() == 4);
    assert(requests[0]["admin_type"] == "racf-options");
    assert(requests[1]["admin_type"] == "racf-rrsf");
    assert(requests[2]["admin_type"] == "group");
    assert(requests[3]["class"] == "CDT     ");

    requests.clear();
    responses[2] = {{"return_codes", {{"sear_return_code", 4}, {"saf_return_code", 8}, {"racf_return_code", 8}, {"racf_reason_code", 0}}}};
    assert(run_command({"racfstats", "--json"}, output) == 1);
    const auto partial = nlohmann::json::parse(output);
    assert(partial["statistics"]["visible_groups"].is_null());
    assert(partial["statistics"]["visible_cdt_profiles"] == 0);
    assert(partial["errors"].contains("groups"));

    requests.clear();
    responses[2] = {{"return_codes", success}, {"profiles", nlohmann::json::array()}};
    assert(run_command({"racfstats", "--no-color"}, output) == 0);
    assert(output.find("  | Visible groups:") != std::string::npos);
    assert(output.find("LOGGING") != std::string::npos);
    assert(output.find("\x1b[") == std::string::npos);
    for (const unsigned char character : output)
        assert(character < 0x80);

    requests.clear();
    assert(run_command({"racfstats"}, output) == 0);
    assert(output.find("\x1b[34mDATABASE\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[33mPASSWORDS & PASSPHRASES\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[32mOPERATIONS\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[35mLOGGING\x1b[0m") != std::string::npos);
    return 0;
}
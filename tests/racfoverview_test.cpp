#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define main racfoverview_main
#include "../racshell/src/commands/racfoverview/racfoverview.cpp"
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
        const int status = racfoverview_main(static_cast<int>(argv.size()), argv.data());
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
        {{"return_codes", success}, {"profile", {{"base", {{"base:max_password_change_interval", 90}, {"base:max_incorrect_password_attempts", 5}, {"base:log_operator_actions", false}, {"base:log_commands_issued_by_special_users", true}, {"base:special_character_password_support", true}, {"base:raclist", {"FACILITY", "CDT"}}}}}}},
        {{"return_codes", success}, {"profile", {{"base", {{"base:subsystem_operator_prefix", "RACF"}, {"base:subsystem_userid", "RACFUSER"}, {"base:number_of_defined_nodes", 3}}}}}},
        {{"return_codes", success}, {"profiles", {"GROUP1", "GROUP2"}}},
        {{"return_codes", empty}, {"profiles", nlohmann::json::array()}}};
    std::string output;
    assert(run_command({"racfoverview", "--json"}, output) == 0);
    const auto parsed = nlohmann::json::parse(output);
    assert(parsed["statistics"]["visible_groups"] == 2);
    assert(parsed["statistics"]["visible_cdt_profiles"] == 0);
    assert(parsed["statistics"]["raclisted_classes"] == 2);
    assert(parsed["statistics"]["max_password_change_interval"] == 90);
    assert(parsed["statistics"]["max_incorrect_password_attempts"] == 5);
    assert(parsed["statistics"]["special_character_password_support"] == true);
    assert(parsed["statistics"]["log_operator_actions"] == false);
    assert(parsed["statistics"]["log_commands_issued_by_special_users"] == true);
    assert(parsed["statistics"]["passphrase_change_interval"].is_null());
    assert(parsed["statistics"]["rrsf_operator_prefix"] == "RACF");
    assert(parsed["statistics"]["rrsf_subsystem_userid"] == "RACFUSER");
    assert(parsed["statistics"]["rrsf_number_of_defined_nodes"] == 3);
    assert(parsed["errors"].empty());
    assert(requests.size() == 4);
    assert(requests[0]["admin_type"] == "racf-options");
    assert(requests[1]["admin_type"] == "racf-rrsf");
    assert(requests[2]["admin_type"] == "group");
    assert(requests[3]["class"] == "CDT     ");

    requests.clear();
    responses[2] = {{"return_codes", {{"sear_return_code", 4}, {"saf_return_code", 8}, {"racf_return_code", 8}, {"racf_reason_code", 0}}}};
    assert(run_command({"racfoverview", "--json"}, output) == 1);
    const auto partial = nlohmann::json::parse(output);
    assert(partial["statistics"]["visible_groups"].is_null());
    assert(partial["statistics"]["visible_cdt_profiles"] == 0);
    assert(partial["errors"].contains("groups"));

    requests.clear();
    responses[2] = {{"return_codes", success}, {"profiles", nlohmann::json::array()}};
    assert(run_command({"racfoverview", "--no-color"}, output) == 0);
    assert(output.find("  | Visible groups:") != std::string::npos);
    assert(output.find("  | RACLISTed classes: 2") != std::string::npos);
    assert(output.find("For all settings use commands such as rrsflist and racfctl list") != std::string::npos);
    assert(output.find("LOGGING") != std::string::npos);
    assert(output.find("\x1b[") == std::string::npos);
    for (const unsigned char character : output)
        assert(character < 0x80);

    requests.clear();
    responses[1]["profile"]["base"]["base:nodes"] = nlohmann::json::array({
        {{"base:node_name", "ACTIVE"}, {"base:node_state", 128}, {"base:is_local_node", true}},
        {{"base:node_name", "REMOTE"}, {"base:node_state", 128}, {"base:is_local_node", false}}});
    assert(run_command({"racfoverview"}, output) == 0);
    assert(output.find("\x1b[36mDATABASE\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[36mPASSWORDS & PASSPHRASES\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[36mOPERATIONS\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[36mLOGGING\x1b[0m") != std::string::npos);
    assert(output.find("\x1b[36mRRSF NODES\x1b[0m\n  \xe2\x94\x8c") != std::string::npos);
    assert(output.find("ACTIVE  |  Operative active \xe2\x94\x82  \xe2\x86\x90 you are here") != std::string::npos);
    assert(output.find("REMOTE  |  Operative active \xe2\x94\x82\n") != std::string::npos);

    requests.clear();
    responses[1]["profile"]["base"]["base:nodes"].push_back(
        {{"base:node_name", "LONGNODE"}, {"base:node_state", 32}});
    assert(run_command({"racfoverview"}, output) == 0);
    const std::string short_content = "ACTIVE  |  Operative active";
    const std::string long_content = "LONGNODE  |  Operative pending verification";
    const std::string padded_short = short_content + std::string(long_content.size() - short_content.size() + 1, ' ');
    assert(output.find(padded_short + "\xe2\x94\x82  \xe2\x86\x90 you are here") != std::string::npos);
    std::string top_border = "  \xe2\x94\x8c";
    for (std::size_t width = 0; width < long_content.size() + 2; ++width)
        top_border += "\xe2\x94\x80";
    top_border += "\xe2\x94\x90";
    const auto first_border = output.find(top_border);
    const auto second_border = output.find(top_border, first_border + top_border.size());
    assert(first_border != std::string::npos && second_border != std::string::npos);
    assert(output.find(top_border, second_border + top_border.size()) != std::string::npos);

    requests.clear();
    auto &nodes = responses[1]["profile"]["base"]["base:nodes"] = nlohmann::json::array();
    for (int index = 0; index < 11; ++index)
        nodes.push_back({{"base:node_name", "NODE" + std::to_string(index)}, {"base:node_state", 128}, {"base:is_local_node", index == 10}});
    assert(run_command({"racfoverview"}, output) == 0);
    assert(output.find("NODE10  |  Operative active \xe2\x94\x82  \xe2\x86\x90 you are here") != std::string::npos);
    assert(output.find("NODE9  |") == std::string::npos);
    assert(output.find("1 more nodes not shown") != std::string::npos);

    requests.clear();
    nodes[10]["base:node_state"] = 1;
    assert(run_command({"racfoverview"}, output) == 0);
    assert(output.find("NODE10  |  Dormant error   \xe2\x94\x82\x1b[0m  \xe2\x86\x90 you are here\x1b[31m") != std::string::npos);

    requests.clear();
    assert(run_command({"racfoverview", "--no-colour"}, output) == 0);
    assert(output.find("you are here") == std::string::npos);
    assert(output.find("\x1b[") == std::string::npos);
    requests.clear();
    responses[0]["profile"]["base"]["base:raclist"] = nlohmann::json::array();
    assert(run_command({"racfoverview", "--json"}, output) == 0);
    assert(nlohmann::json::parse(output)["statistics"]["raclisted_classes"] == 0);
    requests.clear();
    responses[0]["profile"]["base"].erase("base:raclist");
    assert(run_command({"racfoverview", "--json"}, output) == 0);
    assert(nlohmann::json::parse(output)["statistics"]["raclisted_classes"].is_null());
    requests.clear();
    responses[0]["profile"]["base"]["base:raclist"] = "invalid";
    assert(run_command({"racfoverview", "--json"}, output) == 0);
    assert(nlohmann::json::parse(output)["statistics"]["raclisted_classes"].is_null());
    return 0;
}
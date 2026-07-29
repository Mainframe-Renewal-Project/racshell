#ifndef USER_COMPARISON_DATA_HPP
#define USER_COMPARISON_DATA_HPP

#include <nlohmann/json.hpp>
#include <string>

struct UserComparisonSide
{
    std::string userid;
    std::string raw_response_json;
    nlohmann::json response_json;
    nlohmann::json profile;
    nlohmann::json base;
};

struct UserComparisonData
{
    UserComparisonSide left;
    UserComparisonSide right;
    bool compare_groups = false;
    bool compare_security = false;
    bool compare_tso = false;
    bool compare_kerberos = false;
    bool compare_cics = false;
    bool compare_omvs = false;
    bool compare_csdata = false;
    bool raw_json_output = false;
    bool identical = false;
    nlohmann::json differences;
};

#endif // USER_COMPARISON_DATA_HPP
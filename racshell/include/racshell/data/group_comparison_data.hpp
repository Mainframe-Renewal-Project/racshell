#ifndef GROUP_COMPARISON_DATA_HPP
#define GROUP_COMPARISON_DATA_HPP

#include <string>
#include <nlohmann/json.hpp>

struct GroupComparisonSide
{
    std::string groupid;
    std::string raw_response_json;
    nlohmann::json response_json;
    nlohmann::json profile;
    nlohmann::json base;
};

struct GroupComparisonData
{
    GroupComparisonSide left;
    GroupComparisonSide right;
    bool compare_users = false;
    bool compare_omvs = false;
    bool compare_csdata = false;
    bool raw_json_output = false;
    bool identical = false;
    nlohmann::json differences;
};

#endif // GROUP_COMPARISON_DATA_HPP
#ifndef RACF_OPTIONS_DATA_HPP
#define RACF_OPTIONS_DATA_HPP

#include <nlohmann/json.hpp>

struct RacfOptionsData
{
    nlohmann::json profile;
};

#endif
#ifndef RRSF_DATA_HPP
#define RRSF_DATA_HPP

#include <nlohmann/json.hpp>

struct RrsfData
{
    nlohmann::json profile;
};

#endif
#ifndef RESOURCE_DATA_HPP
#define RESOURCE_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "access_entry.hpp"

struct ResourceData
{
    std::string resource;
    std::string resource_class;
    std::string owner;
    std::string uacc;
    nlohmann::json profile;
    nlohmann::json base;
    std::vector<AccessEntry> access_list;
    nlohmann::json csdata;
};

#endif // RESOURCE_DATA_HPP

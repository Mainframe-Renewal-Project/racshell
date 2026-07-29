#ifndef DATASET_DATA_HPP
#define DATASET_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "access_entry.hpp"

struct DatasetData
{
    std::string dataset;
    std::string owner;
    std::string uacc;
    bool audited = false;
    int access_count = 0;
    std::vector<AccessEntry> access_list;
    nlohmann::json csdata;
};

#endif // DATASET_DATA_HPP

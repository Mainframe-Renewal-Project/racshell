#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

#include <string>
#include "include/racshell/data/user_data.hpp"
#include "include/racshell/data/group_data.hpp"
#include "include/racshell/data/dataset_data.hpp"
#include "include/racshell/data/resource_data.hpp"
#include "include/racshell/data/group_comparison_data.hpp"
#include "include/racshell/data/user_comparison_data.hpp"

class OutputFormatter
{
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(const UserData &user) = 0;
    virtual std::string format(const GroupData &group) = 0;
    virtual std::string format(const DatasetData &dataset) = 0;
    virtual std::string format(const ResourceData &resource) = 0;
    virtual std::string format(const GroupComparisonData &comparison) = 0;
    virtual std::string format(const UserComparisonData &comparison) = 0;
};

class TextFormatter : public OutputFormatter
{
public:
    std::string format(const UserData &user) override;
    std::string format(const GroupData &group) override;
    std::string format(const DatasetData &dataset) override;
    std::string format(const ResourceData &resource) override;
    std::string format(const GroupComparisonData &comparison) override;
    std::string format(const UserComparisonData &comparison) override;
};

class JsonFormatter : public OutputFormatter
{
public:
    std::string format(const UserData &user) override;
    std::string format(const GroupData &group) override;
    std::string format(const DatasetData &dataset) override;
    std::string format(const ResourceData &resource) override;
    std::string format(const GroupComparisonData &comparison) override;
    std::string format(const UserComparisonData &comparison) override;
};

#endif // OUTPUT_FORMATTER_HPP

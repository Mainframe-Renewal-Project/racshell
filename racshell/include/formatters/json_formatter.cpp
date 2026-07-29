#include "racshell/output_formatter.hpp"

namespace
{

    void add_if_present(nlohmann::json &output,
                        const nlohmann::json &value,
                        const char *key)
    {
        if (!value.is_null() && !value.empty())
        {
            output[key] = value;
        }
    }

    inline void add_access_list_to_json(nlohmann::json &output, const std::vector<AccessEntry> &access_list)
    {
        if (!access_list.empty())
        {
            nlohmann::json access_array = nlohmann::json::array();
            for (const auto &entry : access_list)
            {
                access_array.push_back({{"access_type", entry.access_type}, {"access_id", entry.access_id}});
            }
            output["access_list"] = access_array;
        }
    }


}
std::string JsonFormatter::format(const UserData &user)
{
    nlohmann::json output;

    output["userid"] = user.userid;
    output["name"] = user.name;
    output["owner"] = user.owner;
    output["created_date"] = user.created_date;
    output["revoked"] = user.revoked;

    if (!user.groups.empty())
    {
        output["groups"] = user.groups;
    }

    add_if_present(output, user.security, "security");
    add_if_present(output, user.tso, "tso");
    add_if_present(output, user.omvs, "omvs");
    add_if_present(output, user.kerberos, "kerberos");
    add_if_present(output, user.cics, "cics");
    add_if_present(output, user.csdata, "csdata");

    return output.dump(2);
}

std::string JsonFormatter::format(const GroupData &group)
{ // TODO: check this through
    nlohmann::json output;

    output["groupid"] = group.groupid;
    output["owner"] = group.owner;
    output["created_date"] = group.created_date;
    output["superior_group"] = group.superior_group;
    output["universal"] = group.universal;
    output["terminal_universal_access"] = group.terminal_universal_access;
    output["installation_data"] = group.installation_data;

    if (!group.connected_users.empty())
    {
        nlohmann::json users = nlohmann::json::array();
        for (const auto &u : group.connected_users)
        {
            users.push_back({{"userid", u.userid}, {"authority", u.authority}});
        }
        output["connected_users"] = users;
    }

    add_if_present(output, group.omvs, "omvs");
    add_if_present(output, group.csdata, "csdata");

    return output.dump(2);
}

std::string JsonFormatter::format(const DatasetData &dataset)
{
    nlohmann::json output;

    output["dataset"] = dataset.dataset;
    output["owner"] = dataset.owner;
    output["uacc"] = dataset.uacc;
    output["audited"] = dataset.audited;
    output["access_count"] = dataset.access_count;

    if (!dataset.access_list.empty())
    {
        nlohmann::json access_array = nlohmann::json::array();
        for (const auto &entry : dataset.access_list)
        {
            access_array.push_back({{"access_type", entry.access_type}, {"access_id", entry.access_id}});
        }
        output["access_list"] = access_array;
    }

    add_if_present(output, dataset.csdata, "csdata");
    add_access_list_to_json(output, dataset.access_list);

    return output.dump(2);
}

std::string JsonFormatter::format(const ResourceData &resource)
{
    nlohmann::json output;

    output["resource"] = resource.resource;
    output["class"] = resource.resource_class;
    output["owner"] = resource.owner;
    output["universal_access"] = resource.uacc;

    if (!resource.access_list.empty())
    {
        nlohmann::json access_array = nlohmann::json::array();
        for (const auto &entry : resource.access_list)
        {
            access_array.push_back({{"access_type", entry.access_type}, {"access_id", entry.access_id}});
        }
        output["access_list"] = access_array;
    }

    add_if_present(output, resource.csdata, "csdata");
    add_access_list_to_json(output, resource.access_list);

    return output.dump(2);
}

std::string JsonFormatter::format(const GroupComparisonData &comparison)
{
        if (comparison.raw_json_output)
        {
            nlohmann::json output = {
                {"left_group", comparison.left.groupid},
                {"right_group", comparison.right.groupid}};

            if (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty())
            {
                output["left_response"] = comparison.left.response_json;
            }
            else
            {
                output["left_response_raw"] = comparison.left.raw_response_json;
            }

            if (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty())
            {
                output["right_response"] = comparison.right.response_json;
            }
            else
            {
                output["right_response_raw"] = comparison.right.raw_response_json;
            }

            return output.dump(2);
        }

        const nlohmann::json output = {
            {"left_group", comparison.left.groupid},
            {"right_group", comparison.right.groupid},
            {"identical", comparison.identical},
            {"differences", comparison.differences}};
        return output.dump(2);
}

std::string JsonFormatter::format(const UserComparisonData &comparison)
{
        if (comparison.raw_json_output)
        {
            nlohmann::json output = {
                {"left_user", comparison.left.userid},
                {"right_user", comparison.right.userid}};

            if (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty())
            {
                output["left_response"] = comparison.left.response_json;
            }
            else
            {
                output["left_response_raw"] = comparison.left.raw_response_json;
            }

            if (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty())
            {
                output["right_response"] = comparison.right.response_json;
            }
            else
            {
                output["right_response_raw"] = comparison.right.raw_response_json;
            }

            return output.dump(2);
        }

        const nlohmann::json output = {
            {"left_user", comparison.left.userid},
            {"right_user", comparison.right.userid},
            {"identical", comparison.identical},
            {"differences", comparison.differences}};
        return output.dump(2);
}

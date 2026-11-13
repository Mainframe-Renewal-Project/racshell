#include <argparse.hpp>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#if __has_include(<sys/ioctl.h>)
#include <sys/ioctl.h>
#include <unistd.h>
#define RACSHELL_HAVE_IOCTL 1
#endif

#include <nlohmann/json.hpp>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"

namespace
{

    namespace tc = racshell::terminal_color;

    constexpr int sibling_gap = 2;

    struct OmvsInfo
    {
        bool present = false;
        nlohmann::json gid = nullptr;
    };

    struct Node
    {
        std::string name;
        std::vector<Node> children;
        int width = 0;
        std::optional<OmvsInfo> omvs;
        std::optional<std::size_t> user_count;
    };

    struct Cell
    {
        std::string glyph;
        std::string_view color;
        std::uint8_t bits = 0;
    };

    constexpr std::uint8_t up = 1, down = 2, left = 4, right = 8;

    // Indexed by the up/down/left/right bit mask.
    const char *const junction_glyphs[16] = {
        " ", "│", "│", "│", "─", "┘", "┐", "┤",
        "─", "└", "┌", "├", "─", "┴", "┬", "┼"};

    class Canvas
    {
    public:
        Canvas(int width, int height)
            : width_(width), cells_(height, std::vector<Cell>(width)) {}

        void put(int x, int y, const std::string &glyph, std::string_view color)
        {
            cells_[y][x] = {glyph, color, 0};
        }

        void line(int x, int y, std::uint8_t bits)
        {
            Cell &cell = cells_[y][x];
            cell.bits |= bits;
            cell.glyph = junction_glyphs[cell.bits];
            cell.color = tc::gray;
        }

        void print(std::ostream &out) const
        {
            for (const auto &row : cells_)
            {
                int last = width_ - 1;
                while (last >= 0 && row[last].glyph.empty())
                {
                    --last;
                }
                std::string_view current;
                for (int x = 0; x <= last; ++x)
                {
                    const Cell &cell = row[x];
                    if (cell.color != current)
                    {
                        if (!current.empty())
                        {
                            racshell::reset_color(out);
                        }
                        if (!cell.color.empty())
                        {
                            racshell::colorize(out, cell.color);
                        }
                        current = cell.color;
                    }
                    out << (cell.glyph.empty() ? " " : cell.glyph);
                }
                if (!current.empty())
                {
                    racshell::reset_color(out);
                }
                out << "\n";
            }
        }

    private:
        int width_;
        std::vector<std::vector<Cell>> cells_;
    };

    // Splits UTF-8 text into one string per code point.
    std::vector<std::string> glyphs_of(const std::string &text)
    {
        std::vector<std::string> glyphs;
        for (const char c : text)
        {
            if ((static_cast<unsigned char>(c) & 0xC0) != 0x80 || glyphs.empty())
            {
                glyphs.emplace_back();
            }
            glyphs.back().push_back(c);
        }
        return glyphs;
    }

    std::vector<std::string> node_lines(const Node &node)
    {
        std::vector<std::string> lines{node.name};
        if (node.omvs && !node.omvs->present)
        {
            lines.emplace_back("no OMVS");
        }
        else if (node.omvs)
        {
            lines.push_back("OMVS GID=" +
                            (node.omvs->gid.is_null() ? "?" : node.omvs->gid.dump()));
        }
        if (node.user_count)
        {
            lines.push_back("users=" +
                            (node.user_count.value() > 100 ? "100+" : std::to_string(node.user_count.value())));
        }
        return lines;
    }

    int box_width(const Node &node)
    {
        int width = 0;
        for (const auto &line : node_lines(node))
        {
            width = std::max(width, static_cast<int>(glyphs_of(line).size()));
        }
        return width + 4;
    }

    int box_height(const Node &node)
    {
        return static_cast<int>(node_lines(node).size()) + 2;
    }

    int tree_depth(const Node &node)
    {
        int deepest = 0;
        for (const auto &child : node.children)
        {
            deepest = std::max(deepest, tree_depth(child));
        }
        return deepest + 1;
    }

    int max_box_height(const Node &node)
    {
        int height = box_height(node);
        for (const auto &child : node.children)
        {
            height = std::max(height, max_box_height(child));
        }
        return height;
    }

    void measure(Node &node)
    {
        int children_width = 0;
        for (auto &child : node.children)
        {
            measure(child);
            children_width += child.width;
        }
        if (!node.children.empty())
        {
            children_width += sibling_gap * static_cast<int>(node.children.size() - 1);
        }
        node.width = std::max(box_width(node), children_width);
    }

    void draw_box(Canvas &canvas, const Node &node, int box_left, int top)
    {
        const int box_w = box_width(node);
        const int box_h = box_height(node);
        canvas.put(box_left, top, "┌", tc::blue);
        canvas.put(box_left + box_w - 1, top, "┐", tc::blue);
        canvas.put(box_left, top + box_h - 1, "└", tc::blue);
        canvas.put(box_left + box_w - 1, top + box_h - 1, "┘", tc::blue);
        for (int x = box_left + 1; x < box_left + box_w - 1; ++x)
        {
            canvas.put(x, top, "─", tc::blue);
            canvas.put(x, top + box_h - 1, "─", tc::blue);
        }
        const std::vector<std::string> lines = node_lines(node);
        for (std::size_t line_index = 0; line_index < lines.size(); ++line_index)
        {
            const auto glyphs = glyphs_of(lines[line_index]);
            std::string_view color = tc::cyan;
            if (lines[line_index] == "no OMVS")
            {
                color = tc::red;
            }
            else if (node.user_count && node.user_count.value() == 0 && line_index + 1 == lines.size())
            {
                color = tc::yellow;
            }
            const int row = top + 1 + static_cast<int>(line_index);
            canvas.put(box_left, row, "│", tc::blue);
            canvas.put(box_left + box_w - 1, row, "│", tc::blue);
            for (std::size_t i = 0; i < glyphs.size(); ++i)
            {
                canvas.put(box_left + 2 + static_cast<int>(i), row, glyphs[i], color);
            }
        }
    }

    // Draws the subtree into [origin, origin + node.width) and returns the box center column.
    int place(const Node &node, int origin, int depth, int level_step, Canvas &canvas)
    {
        const int box_w = box_width(node);
        const int box_h = box_height(node);
        const int top = depth * level_step;
        int box_left;

        std::vector<int> child_centers;
        if (node.children.empty())
        {
            box_left = origin + (node.width - box_w) / 2;
        }
        else
        {
            int children_width = sibling_gap * static_cast<int>(node.children.size() - 1);
            for (const auto &child : node.children)
            {
                children_width += child.width;
            }
            int x = origin + (node.width - children_width) / 2;
            for (const auto &child : node.children)
            {
                child_centers.push_back(place(child, x, depth + 1, level_step, canvas));
                x += child.width + sibling_gap;
            }
            const int mid = (child_centers.front() + child_centers.back()) / 2;
            box_left = std::clamp(mid - box_w / 2, origin, origin + node.width - box_w);
        }

        const int center = box_left + box_w / 2;

        draw_box(canvas, node, box_left, top);

        if (depth > 0)
        {
            canvas.put(center, top, "┴", tc::blue);
        }

        if (!child_centers.empty())
        {
            canvas.put(center, top + box_h - 1, "┬", tc::blue);

            const int bus_row = top + box_h;
            const int child_top = (depth + 1) * level_step;
            const int bus_left = std::min(center, child_centers.front());
            const int bus_right = std::max(center, child_centers.back());
            for (int x = bus_left; x <= bus_right; ++x)
            {
                std::uint8_t bits = 0;
                if (x > bus_left)
                {
                    bits |= left;
                }
                if (x < bus_right)
                {
                    bits |= right;
                }
                canvas.line(x, bus_row, bits);
            }
            canvas.line(center, bus_row, up);
            for (int child_center : child_centers)
            {
                canvas.line(child_center, bus_row, down);
                for (int row = bus_row + 1; row < child_top; ++row)
                {
                    canvas.line(child_center, row, up | down);
                }
            }
        }

        return center;
    }

    constexpr int indent_step = 3;

    int vertical_height(const Node &node)
    {
        int height = box_height(node);
        for (const auto &child : node.children)
        {
            height += vertical_height(child);
        }
        return height;
    }

    int vertical_width(const Node &node, int left_edge)
    {
        int widest = left_edge + box_width(node);
        for (const auto &child : node.children)
        {
            widest = std::max(widest, vertical_width(child, left_edge + indent_step));
        }
        return widest;
    }

    // Children hang below their parent, each indented and joined by a spine.
    void place_vertical(const Node &node, int left_edge, int top, Canvas &canvas)
    {
        draw_box(canvas, node, left_edge, top);

        const int spine = left_edge + 1;
        int row = top + box_height(node);
        if (!node.children.empty())
        {
            canvas.put(spine, top + box_height(node) - 1, "┬", tc::blue);
        }
        for (std::size_t i = 0; i < node.children.size(); ++i)
        {
            const bool is_last = i + 1 == node.children.size();
            const int child_rows = vertical_height(node.children[i]);

            if (is_last)
            {
                canvas.line(spine, row, up | down);
                canvas.line(spine, row + 1, up | right);
            }
            else
            {
                for (int y = row; y < row + child_rows; ++y)
                {
                    canvas.line(spine, y, up | down);
                }
                canvas.line(spine, row + 1, right);
            }
            canvas.line(spine + 1, row + 1, left | right);

            place_vertical(node.children[i], left_edge + indent_step, row, canvas);
            canvas.put(left_edge + indent_step, row + 1, "┤", tc::blue);
            row += child_rows;
        }
    }

    // Redirected output has no terminal to measure, so it is left unlimited.
    int terminal_columns()
    {
#ifdef RACSHELL_HAVE_IOCTL
        winsize size{};
        if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0)
        {
            return size.ws_col;
        }
#endif
        return std::numeric_limits<int>::max();
    }

    nlohmann::json to_json(const Node &node)
    {
        nlohmann::json children = nlohmann::json::array();
        for (const auto &child : node.children)
        {
            children.push_back(to_json(child));
        }
        nlohmann::json result = {{"group", node.name}, {"subgroups", children}};
        if (node.omvs)
        {
            result["omvs"] = node.omvs->present;
            result["gid"] = node.omvs->gid;
        }
        if (node.user_count)
        {
            result["user_count"] = node.user_count.value() > 100
                                        ? nlohmann::json("100+")
                                        : nlohmann::json(node.user_count.value());
        }
        return result;
    }

    void build_subtree(Node &node,
                       const std::map<std::string, std::vector<std::string>> &children_of,
                       const std::map<std::string, OmvsInfo> &omvs_by_group,
                       const std::map<std::string, std::size_t> &users_by_group,
                       std::set<std::string> &visited,
                       int depth,
                       int max_depth)
    {
        if (max_depth > 0 && depth >= max_depth)
        {
            return;
        }
        const auto it = children_of.find(node.name);
        if (it == children_of.end())
        {
            return;
        }
        for (const auto &name : it->second)
        {
            if (!visited.insert(name).second)
            {
                continue;
            }
            Node child;
            child.name = name;
            const auto omvs = omvs_by_group.find(name);
            if (omvs != omvs_by_group.end())
            {
                child.omvs = omvs->second;
            }
            const auto users = users_by_group.find(name);
            if (users != users_by_group.end())
            {
                child.user_count = users->second;
            }
            build_subtree(child, children_of, omvs_by_group, users_by_group, visited, depth + 1, max_depth);
            node.children.push_back(std::move(child));
        }
    }

    OmvsInfo get_omvs_info(const nlohmann::json &profile)
    {
        OmvsInfo result;
        const auto segment = profile.find("omvs");
        if (segment != profile.end() && segment->is_object())
        {
            result.present = true;
            const auto gid = segment->find("omvs:gid");
            if (gid != segment->end() && gid->is_number_integer())
            {
                result.gid = *gid;
            }
        }
        return result;
    }

    std::size_t get_user_count(const nlohmann::json &base)
    {
        const auto users = base.find("base:connected_users");
        return users != base.end() && users->is_array() ? users->size() : 0;
    }

    bool fetch_group_names(std::vector<std::string> &names, bool debug, std::string &error)
    {
        const std::string request = nlohmann::json{{"operation", "search"}, {"admin_type", "group"}}.dump();
        sear_result_t *result = sear(request.c_str(), request.length(), debug);

        nlohmann::json response;
        if (!racshell::parse_sear_response_json(result->result_json, response, error))
        {
            return false;
        }
        if (!racshell::validate_sear_return_codes(racshell::extract_sear_return_codes(response), error))
        {
            return false;
        }
        if (response.contains("profiles") && response["profiles"].is_array())
        {
            for (const auto &profile : response["profiles"])
            {
                if (profile.is_string())
                {
                    names.push_back(racshell::from_racf_identifier_text(profile.get<std::string>()));
                }
            }
        }
        return true;
    }

    bool fetch_superior(const std::string &name, bool debug, std::string &superior,
                        OmvsInfo *omvs, std::size_t *user_count, std::string &error)
    {
        const std::string request = nlohmann::json{
            {"operation", "extract"},
            {"admin_type", "group"},
            {"group", racshell::to_racf_identifier_text(name)}}.dump();
        sear_result_t *result = sear(request.c_str(), request.length(), debug);

        const racshell::SearResponseInfo info = racshell::validate_sear_response(result->result_json, "group");
        if (!info.success)
        {
            error = name + ": " + info.error_message;
            return false;
        }
        superior.clear();
        racshell::assign_string(info.base, "base:superior_group", superior);
        superior = racshell::from_racf_identifier_text(superior);
        if (omvs)
        {
            *omvs = get_omvs_info(info.profile);
        }
        if (user_count)
        {
            *user_count = get_user_count(info.base);
        }
        return true;
    }

    bool fetch_subtree(Node &node, bool debug, bool show_omvs, bool show_users,
                       std::set<std::string> &visited,
                       int depth, int max_depth, std::string &error)
    {
        if (!show_omvs && !show_users && max_depth > 0 && depth >= max_depth)
        {
            return true;
        }

        const std::string request = nlohmann::json{
            {"operation", "extract"},
            {"admin_type", "group"},
            {"group", racshell::to_racf_identifier_text(node.name)}}.dump();
        sear_result_t *result = sear(request.c_str(), request.length(), debug);
        const racshell::SearResponseInfo info = racshell::validate_sear_response(result->result_json, "group");
        if (!info.success)
        {
            error = node.name + ": " + info.error_message;
            return false;
        }

        if (show_omvs)
        {
            node.omvs = get_omvs_info(info.profile);
        }
        if (show_users)
        {
            node.user_count = get_user_count(info.base);
        }
        if (max_depth > 0 && depth >= max_depth)
        {
            return true;
        }

        std::vector<std::string> names;
        const auto subgroups = info.base.find("base:subgroups");
        if (subgroups != info.base.end() && subgroups->is_array())
        {
            for (const auto &subgroup : *subgroups)
            {
                std::string name;
                racshell::assign_string(subgroup, "base:subgroup", name);
                name = racshell::from_racf_identifier_text(name);
                if (!name.empty())
                {
                    names.push_back(std::move(name));
                }
            }
        }
        std::sort(names.begin(), names.end());
        for (const auto &name : names)
        {
            if (!visited.insert(name).second)
            {
                continue;
            }
            Node child;
            child.name = name;
            if (!fetch_subtree(child, debug, show_omvs, show_users, visited, depth + 1, max_depth, error))
            {
                return false;
            }
            node.children.push_back(std::move(child));
        }
        return true;
    }

} // namespace

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("groupchart");

    program.add_argument("root")
        .help("group to use as the top of the chart; defaults to every top-level group")
        .default_value(std::string(""));

    program.add_argument("-l", "--levels")
        .help("maximum number of levels to show below the root (0 for unlimited)")
        .scan<'i', int>()
        .default_value(0);

    program.add_argument("-w", "--width")
        .help("maximum chart width before switching to the vertical layout (0 for terminal width)")
        .scan<'i', int>()
        .default_value(0);

    racshell::add_toggle_argument(program, "-v", "--vertical", "stack subgroups vertically (automatic when the chart is wider than the terminal)");
    racshell::add_toggle_argument(program, "-o", "--omvs", "show whether each group has an OMVS segment and its GID");
    racshell::add_toggle_argument(program, "-u", "--users", "show connected user count for each group (capped at 100+)");
    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        racshell::print_error(std::cerr, err.what());
        std::cerr << program;
        return 1;
    }

    racshell::set_color_output_enabled(!program.get<bool>("no-color"));

    std::string root_name = program.get<std::string>("root");
    std::transform(root_name.begin(), root_name.end(), root_name.begin(),
                   [](unsigned char c)
                   { return static_cast<char>(std::toupper(c)); });
    const int levels = program.get<int>("levels");
    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool show_omvs = program.get<bool>("omvs");
    const bool show_users = program.get<bool>("users");
    const bool force_vertical = program.get<bool>("vertical");
    const int max_width = program.get<int>("width") > 0 ? program.get<int>("width") : terminal_columns();

    if (root_name.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group name");
        return 1;
    }
    if (levels < 0)
    {
        racshell::print_error(std::cerr, "Invalid input, levels must not be negative");
        return 1;
    }

    root_name = racshell::from_racf_identifier_text(root_name);
    std::string error;
    std::vector<Node> trees;
    std::set<std::string> visited;
    if (!root_name.empty())
    {
        Node root;
        root.name = root_name;
        visited.insert(root_name);
        if (!fetch_subtree(root, debug, show_omvs, show_users, visited, 0, levels, error))
        {
            racshell::print_error(std::cerr, error);
            return 1;
        }
        trees.push_back(std::move(root));
    }
    else
    {
        std::vector<std::string> groups;
        if (!fetch_group_names(groups, debug, error))
        {
            racshell::print_error(std::cerr, error);
            return 1;
        }

        const std::set<std::string> known(groups.begin(), groups.end());
        std::map<std::string, std::vector<std::string>> children_of;
        std::map<std::string, OmvsInfo> omvs_by_group;
        std::map<std::string, std::size_t> users_by_group;
        std::vector<std::string> roots;
        for (const auto &name : groups)
        {
            std::string superior;
            OmvsInfo omvs;
            std::size_t user_count = 0;
            if (!fetch_superior(name, debug, superior, show_omvs ? &omvs : nullptr,
                                show_users ? &user_count : nullptr, error))
            {
                racshell::print_error(std::cerr, error);
                return 1;
            }
            if (show_omvs)
            {
                omvs_by_group.emplace(name, std::move(omvs));
            }
            if (show_users)
            {
                users_by_group.emplace(name, user_count);
            }
            if (superior.empty() || superior == name || known.find(superior) == known.end())
            {
                roots.push_back(name);
            }
            else
            {
                children_of[superior].push_back(name);
            }
        }

        std::sort(roots.begin(), roots.end());
        for (auto &entry : children_of)
        {
            std::sort(entry.second.begin(), entry.second.end());
        }
        if (roots.empty())
        {
            std::cout << "No groups found\n";
            return 0;
        }
        for (const auto &name : roots)
        {
            if (!visited.insert(name).second)
            {
                continue;
            }
            Node root;
            root.name = name;
            const auto omvs = omvs_by_group.find(name);
            if (omvs != omvs_by_group.end())
            {
                root.omvs = omvs->second;
            }
            const auto users = users_by_group.find(name);
            if (users != users_by_group.end())
            {
                root.user_count = users->second;
            }
            build_subtree(root, children_of, omvs_by_group, users_by_group, visited, 0, levels);
            trees.push_back(std::move(root));
        }
    }

    if (json_output)
    {
        nlohmann::json output = nlohmann::json::array();
        for (const auto &tree : trees)
        {
            output.push_back(to_json(tree));
        }
        std::cout << output.dump(2) << "\n";
        return 0;
    }

    for (std::size_t i = 0; i < trees.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << "\n";
        }
        measure(trees[i]);
        if (force_vertical || trees[i].width > max_width)
        {
            Canvas canvas(vertical_width(trees[i], 0), vertical_height(trees[i]));
            place_vertical(trees[i], 0, 0, canvas);
            canvas.print(std::cout);
            continue;
        }
        const int level_step = max_box_height(trees[i]) + 2;
        const int height = (tree_depth(trees[i]) - 1) * level_step + max_box_height(trees[i]);
        Canvas canvas(trees[i].width, height);
        place(trees[i], 0, 0, level_step, canvas);
        canvas.print(std::cout);
    }

    return 0;
}

#include <argparse.hpp>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
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
    constexpr int rows_per_level = 5;

    struct Node
    {
        std::string name;
        std::vector<Node> children;
        int width = 0;
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

    int box_width(const Node &node)
    {
        return static_cast<int>(glyphs_of(node.name).size()) + 4;
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
        canvas.put(box_left, top, "┌", tc::blue);
        canvas.put(box_left + box_w - 1, top, "┐", tc::blue);
        canvas.put(box_left, top + 2, "└", tc::blue);
        canvas.put(box_left + box_w - 1, top + 2, "┘", tc::blue);
        canvas.put(box_left, top + 1, "│", tc::blue);
        canvas.put(box_left + box_w - 1, top + 1, "│", tc::blue);
        for (int x = box_left + 1; x < box_left + box_w - 1; ++x)
        {
            canvas.put(x, top, "─", tc::blue);
            canvas.put(x, top + 2, "─", tc::blue);
        }
        const std::vector<std::string> glyphs = glyphs_of(node.name);
        for (std::size_t i = 0; i < glyphs.size(); ++i)
        {
            canvas.put(box_left + 2 + static_cast<int>(i), top + 1, glyphs[i], tc::cyan);
        }
    }

    // Draws the subtree into [origin, origin + node.width) and returns the box center column.
    int place(const Node &node, int origin, int depth, Canvas &canvas)
    {
        const int box_w = box_width(node);
        const int top = depth * rows_per_level;
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
                child_centers.push_back(place(child, x, depth + 1, canvas));
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
            canvas.put(center, top + 2, "┬", tc::blue);

            const int bus_row = top + 3;
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
                canvas.line(child_center, bus_row + 1, up | down);
            }
        }

        return center;
    }

    constexpr int indent_step = 3;

    int vertical_height(const Node &node)
    {
        int height = 3;
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
        int row = top + 3;
        if (!node.children.empty())
        {
            canvas.put(spine, top + 2, "┬", tc::blue);
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
        return {{"group", node.name}, {"subgroups", children}};
    }

    void build_subtree(Node &node,
                       const std::map<std::string, std::vector<std::string>> &children_of,
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
            build_subtree(child, children_of, visited, depth + 1, max_depth);
            node.children.push_back(std::move(child));
        }
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

    bool fetch_superior(const std::string &name, bool debug, std::string &superior, std::string &error)
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
        return true;
    }

    bool fetch_subtree(Node &node, bool debug, std::set<std::string> &visited,
                       int depth, int max_depth, std::string &error)
    {
        if (max_depth > 0 && depth >= max_depth)
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
            if (!fetch_subtree(child, debug, visited, depth + 1, max_depth, error))
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
        if (!fetch_subtree(root, debug, visited, 0, levels, error))
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
        std::vector<std::string> roots;
        for (const auto &name : groups)
        {
            std::string superior;
            if (!fetch_superior(name, debug, superior, error))
            {
                racshell::print_error(std::cerr, error);
                return 1;
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
            build_subtree(root, children_of, visited, 0, levels);
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
        Canvas canvas(trees[i].width, tree_depth(trees[i]) * rows_per_level - 2);
        place(trees[i], 0, 0, canvas);
        canvas.print(std::cout);
    }

    return 0;
}

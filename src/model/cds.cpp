#include "cds.hpp"
#include <vector>
#include <sstream>

namespace cds
{

    Root create()
    {
        Root r;
        r.ptr = std::make_unique<Node>();
        r.ptr->children = std::make_unique<std::map<std::string, std::unique_ptr<Node>>>();
        return r;
    }

    void load(std::unique_ptr<Node> *root_ref, const std::string &text)
    {
        Node *root = root_ref->get();
        std::vector<Node *> level_father;
        level_father.push_back(root);

        std::istringstream stream(text);
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty())
                continue; // 空行跳过

            // 1. 数缩进，得 level
            long long level = 0;
            while (level * 4 < (long long)line.size() && line[level * 4] == ' ')
                level++;

            std::string sstr = line.substr(level * 4);
            if (sstr.empty())
                continue; // 全是空格的行，跳过（用户的事，但空串后面没法处理）

            // 2. 找空格，区分叶子/目录
            size_t space_pos = sstr.find(' ');
            std::string name, value;
            bool is_leaf;

            if (space_pos == std::string::npos)
            {
                // 没空格 → 目录
                is_leaf = false;
                name = sstr;
            }
            else
            {
                // 有空格 → 叶子
                is_leaf = true;
                name = sstr.substr(0, space_pos);
                value = sstr.substr(space_pos + 1);
            }

            // 3. 找父节点：level 层的父是 level_father[level]
            //    越界或为 nullptr（跳级）→ 归 root
            Node *parent = root;
            if (level < (long long)level_father.size() && level_father[level] != nullptr)
                parent = level_father[level];

            // 4. 在父节点下复用或新建子节点
            auto &slot = (*parent->children)[name];
            if (!slot)
            {
                slot = std::make_unique<Node>();
                slot->name = name;
            }

            // 5. 按叶子/目录分别处理
            if (is_leaf)
            {
                slot->value = value; // 叶子：设值，不动 children
            }
            else
            {
                // 目录：确保 children 存在
                if (!slot->children)
                    slot->children = std::make_unique<std::map<std::string, std::unique_ptr<Node>>>();

                // 关键：把这个目录写进 level_father[level + 1]，供下一层当父
                if (level + 1 >= (long long)level_father.size())
                    level_father.resize(level + 2, nullptr);
                level_father[level + 1] = slot.get();
            }
        }
    }

    namespace
    {
        void generate_node(const Node &node, int depth, std::string &out)
        {
            std::string indent(depth * 4, ' ');

            for (const auto &[name, child] : *node.children)
            {
                out += indent;
                out += name;

                if (!child->children)
                {
                    out += ' ';
                    out += child->value;
                }
                out += '\n';

                if (child->children)
                    generate_node(*child, depth + 1, out);
            }
        }
    }

    std::string get_cds_tree(const std::unique_ptr<Node> *root)
    {
        std::string out = "";
        generate_node(**root, 0, out);
        return out;
    }

    std::unique_ptr<Node> *get_child_by_name(std::unique_ptr<Node> *root_ref, const std::string &name)
    {
        auto it = (*root_ref)->children->find(name);
        if (it == (*root_ref)->children->end())
            return nullptr;
        return &it->second;
    }

    std::string fetch(const std::unique_ptr<Node> *root, std::initializer_list<std::string> names)
    {
        const std::unique_ptr<Node> *cur = root;
        for (const auto &name : names)
        {
            if (!cur)
                break;
            auto it = (*cur)->children->find(name);
            cur = (it == (*cur)->children->end()) ? nullptr : &it->second;
        }
        return cur ? (*cur)->value : "";
    }

    std::unique_ptr<Node> *fetch_node(std::unique_ptr<Node> *root, std::initializer_list<std::string> names)
    {
        std::unique_ptr<Node> *cur = root;
        for (const auto &name : names)
        {
            if (!cur)
                return nullptr;
            auto it = (*cur)->children->find(name);
            if (it == (*cur)->children->end())
                return nullptr;
            cur = &it->second;
        }
        return cur;
    }

    std::vector<std::string> get_children_names(const std::unique_ptr<Node> *node)
    {
        std::vector<std::string> names;
        if (!node || !(*node)->children)
            return names;
        for (const auto &[name, _] : *(*node)->children)
            names.push_back(name);
        return names;
    }
}
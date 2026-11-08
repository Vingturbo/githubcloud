#pragma once
#include <string>
#include <map>
#include <memory>
#include <initializer_list>
#include <vector>

namespace cds {
    struct Node
    {
        std::string name;
        std::string value;
        std::unique_ptr<std::map<std::string, std::unique_ptr<Node>>> children;
    };
    struct Root
    {
        std::unique_ptr<Node> ptr;
        operator std::unique_ptr<Node>*() { return &ptr; }
        operator const std::unique_ptr<Node>*() const { return &ptr; }
    };
    Root create();
    void load(std::unique_ptr<Node> *root, const std::string &text);
    std::string get_cds_tree(const std::unique_ptr<Node> *root);
    std::unique_ptr<Node>* get_child_by_name(std::unique_ptr<Node> *root_ref, const std::string &name);
    std::string fetch(const std::unique_ptr<Node>* root, std::initializer_list<std::string> names);
    std::unique_ptr<Node>* fetch_node(std::unique_ptr<Node> *root, std::initializer_list<std::string> names);
    std::vector<std::string> get_children_names(const std::unique_ptr<Node> *node);
}
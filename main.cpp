#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <list>

// B+ Tree implementation with simplified node structure
// Using map to maintain sorted order

const int MAX_KEYS = 64;
const int MIN_KEYS = 32;

struct BPTNode {
    bool is_leaf;
    int key_count;
    std::multimap<std::string, int> data;  // Allows duplicate keys with different values
    std::map<std::string, BPTNode*> children;
    BPTNode* next;

    BPTNode(bool leaf = true) : is_leaf(leaf), key_count(0), next(nullptr) {}
};

class BPTree {
private:
    std::list<BPTNode> nodes;
    BPTNode* root;

    BPTNode* createNode(bool is_leaf) {
        nodes.emplace_back(is_leaf);
        return &nodes.back();
    }

    BPTNode* findLeaf(BPTNode* node, const std::string& key) {
        if (node->is_leaf) return node;

        auto it = node->children.upper_bound(key);
        if (it == node->children.begin()) {
            if (!node->children.empty()) {
                return findLeaf(node->children.begin()->second, key);
            }
            return nullptr;
        }
        return findLeaf((--it)->second, key);
    }

public:
    BPTree() : root(nullptr) {}

    void insert(const std::string& key, int value) {
        if (!root) {
            root = createNode(true);
            root->data.insert({key, value});
            root->key_count = 1;
            return;
        }

        BPTNode* leaf = findLeaf(root, key);
        if (!leaf) {
            leaf = root;
        }

        // Check for duplicate value
        auto range = leaf->data.equal_range(key);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == value) return;
        }

        leaf->data.insert({key, value});
        leaf->key_count++;

        // Simple split if too large
        if (leaf->key_count > MAX_KEYS) {
            BPTNode* new_leaf = createNode(true);

            int count = 0;
            auto split_it = leaf->data.begin();
            while (count < MIN_KEYS && split_it != leaf->data.end()) {
                ++count;
                ++split_it;
            }

            new_leaf->data.insert(split_it, leaf->data.end());
            leaf->data.erase(split_it, leaf->data.end());

            leaf->key_count = leaf->data.size();
            new_leaf->key_count = new_leaf->data.size();

            new_leaf->next = leaf->next;
            leaf->next = new_leaf;
        }
    }

    void remove(const std::string& key, int value) {
        if (!root) return;

        BPTNode* leaf = findLeaf(root, key);
        if (!leaf) leaf = root;

        auto range = leaf->data.equal_range(key);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == value) {
                leaf->data.erase(it);
                leaf->key_count--;
                break;
            }
        }
    }

    std::vector<int> find(const std::string& key) {
        std::vector<int> result;
        if (!root) return result;

        BPTNode* leaf = findLeaf(root, key);
        if (!leaf) leaf = root;

        // Find all occurrences in all leaves
        BPTNode* current = leaf;
        while (current) {
            for (const auto& pair : current->data) {
                if (pair.first == key) {
                    result.push_back(pair.second);
                } else if (pair.first > key) {
                    break;
                }
            }
            current = current->next;
            if (current && !current->data.empty() && current->data.begin()->first > key) {
                break;
            }
        }

        std::sort(result.begin(), result.end());
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    BPTree tree;

    int n;
    std::cin >> n;

    for (int i = 0; i < n; i++) {
        std::string command;
        std::cin >> command;

        if (command == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            tree.insert(key, value);
        } else if (command == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            tree.remove(key, value);
        } else if (command == "find") {
            std::string key;
            std::cin >> key;
            std::vector<int> values = tree.find(key);

            if (values.empty()) {
                std::cout << "null\n";
            } else {
                for (size_t j = 0; j < values.size(); j++) {
                    if (j > 0) std::cout << " ";
                    std::cout << values[j];
                }
                std::cout << "\n";
            }
        }
    }

    return 0;
}
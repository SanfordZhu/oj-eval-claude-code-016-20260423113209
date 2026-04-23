#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <memory>

// B+ Tree implementation for file storage simulation
// Since actual file I/O isn't required across runs, we simulate with in-memory structure

const int MAX_KEYS = 128;  // Maximum keys per node
const int MIN_KEYS = 64;   // Minimum keys per node (except root)

class BPTreeNode {
public:
    bool is_leaf;
    int key_count;
    std::string keys[MAX_KEYS];
    int values[MAX_KEYS];  // Only used for leaf nodes
    BPTreeNode* children[MAX_KEYS + 1];  // Only used for internal nodes
    BPTreeNode* next;  // For linking leaf nodes
    BPTreeNode* parent;

    BPTreeNode(bool leaf = false) : is_leaf(leaf), key_count(0), next(nullptr), parent(nullptr) {
        for (int i = 0; i <= MAX_KEYS; i++) {
            children[i] = nullptr;
            if (i < MAX_KEYS) {
                values[i] = 0;
            }
        }
    }
};

class BPTree {
private:
    BPTreeNode* root;

    // Find the leaf node that should contain the key
    BPTreeNode* findLeaf(BPTreeNode* node, const std::string& key) {
        if (node->is_leaf) {
            return node;
        }

        int i = 0;
        while (i < node->key_count && node->keys[i] <= key) {
            i++;
        }
        return findLeaf(node->children[i], key);
    }

    // Split a full node
    void splitNode(BPTreeNode* parent, int index) {
        BPTreeNode* full_node = parent->children[index];
        BPTreeNode* new_node = new BPTreeNode(full_node->is_leaf);
        new_node->parent = parent;

        int mid = MIN_KEYS;

        // Copy the right half to new node
        for (int i = mid; i < full_node->key_count; i++) {
            new_node->keys[i - mid] = full_node->keys[i];
            if (full_node->is_leaf) {
                new_node->values[i - mid] = full_node->values[i];
            } else {
                new_node->children[i - mid] = full_node->children[i];
                if (new_node->children[i - mid]) {
                    new_node->children[i - mid]->parent = new_node;
                }
            }
        }

        if (!full_node->is_leaf) {
            new_node->children[new_node->key_count] = full_node->children[full_node->key_count];
            if (new_node->children[new_node->key_count]) {
                new_node->children[new_node->key_count]->parent = new_node;
            }
        }

        new_node->key_count = full_node->key_count - mid;
        full_node->key_count = mid;

        // Update next pointer for leaf nodes
        if (full_node->is_leaf) {
            new_node->next = full_node->next;
            full_node->next = new_node;
        }

        // Insert new child into parent
        for (int i = parent->key_count; i >= index + 1; i--) {
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[index + 1] = new_node;

        // Move keys in parent
        for (int i = parent->key_count - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
        }

        // Copy up the middle key
        parent->keys[index] = full_node->keys[mid];
        parent->key_count++;
    }

    // Insert into a non-full node
    void insertNonFull(BPTreeNode* node, const std::string& key, int value) {
        if (node->is_leaf) {
            // Check for duplicate value
            for (int i = 0; i < node->key_count; i++) {
                if (node->keys[i] == key && node->values[i] == value) {
                    return;  // Duplicate found
                }
            }

            // Find insertion position
            int i = node->key_count - 1;
            while (i >= 0 && (node->keys[i] > key || (node->keys[i] == key && node->values[i] > value))) {
                node->keys[i + 1] = node->keys[i];
                node->values[i + 1] = node->values[i];
                i--;
            }

            node->keys[i + 1] = key;
            node->values[i + 1] = value;
            node->key_count++;
        } else {
            // Find the child to insert into
            int i = node->key_count - 1;
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }
            i++;

            if (node->children[i]->key_count == MAX_KEYS) {
                splitNode(node, i);
                if (node->keys[i] < key) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key, value);
        }
    }

public:
    BPTree() : root(nullptr) {}

    ~BPTree() {
        if (root) {
            deleteSubtree(root);
        }
    }

    void deleteSubtree(BPTreeNode* node) {
        if (!node->is_leaf) {
            for (int i = 0; i <= node->key_count; i++) {
                if (node->children[i]) {
                    deleteSubtree(node->children[i]);
                }
            }
        }
        delete node;
    }

    void insert(const std::string& key, int value) {
        if (!root) {
            root = new BPTreeNode(true);
            root->keys[0] = key;
            root->values[0] = value;
            root->key_count = 1;
            return;
        }

        if (root->key_count == MAX_KEYS) {
            BPTreeNode* new_root = new BPTreeNode(false);
            new_root->children[0] = root;
            root->parent = new_root;
            splitNode(new_root, 0);

            int i = 0;
            if (new_root->keys[0] < key) {
                i++;
            }
            insertNonFull(new_root->children[i], key, value);
            root = new_root;
        } else {
            insertNonFull(root, key, value);
        }
    }

    void remove(const std::string& key, int value) {
        if (!root) return;

        BPTreeNode* leaf = findLeaf(root, key);
        if (!leaf) return;

        // Find and remove the key-value pair
        int idx = -1;
        for (int i = 0; i < leaf->key_count; i++) {
            if (leaf->keys[i] == key && leaf->values[i] == value) {
                idx = i;
                break;
            }
        }

        if (idx == -1) return;  // Not found

        // Shift remaining elements
        for (int i = idx + 1; i < leaf->key_count; i++) {
            leaf->keys[i - 1] = leaf->keys[i];
            leaf->values[i - 1] = leaf->values[i];
        }
        leaf->key_count--;
    }

    std::vector<int> find(const std::string& key) {
        std::vector<int> result;

        if (!root) {
            return result;
        }

        BPTreeNode* leaf = findLeaf(root, key);
        if (!leaf) {
            return result;
        }

        // Search in this leaf and subsequent linked leaves
        BPTreeNode* current = leaf;
        while (current) {
            for (int i = 0; i < current->key_count; i++) {
                if (current->keys[i] == key) {
                    result.push_back(current->values[i]);
                } else if (current->keys[i] > key) {
                    // Since keys are sorted, no more matches possible
                    std::sort(result.begin(), result.end());
                    return result;
                }
            }
            current = current->next;
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
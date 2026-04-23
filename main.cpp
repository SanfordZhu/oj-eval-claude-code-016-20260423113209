#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>

const int MAX_KEYS = 128;  // Maximum keys per node
const int MIN_KEYS = 64;   // Minimum keys per node (except root)

// Node structure for B+ tree
struct Node {
    bool is_leaf;
    int key_count;
    std::string keys[MAX_KEYS];
    int values[MAX_KEYS];
    Node* children[MAX_KEYS + 1];
    Node* next;  // For leaf node linking

    Node(bool leaf = false) : is_leaf(leaf), key_count(0), next(nullptr) {
        for (int i = 0; i <= MAX_KEYS; i++) {
            children[i] = nullptr;
        }
    }
};

class BPTree {
private:
    Node* root;
    std::string filename;

    // Split a node when it's full
    void splitChild(Node* parent, int index, Node* full_child) {
        Node* new_node = new Node(full_child->is_leaf);
        int mid = MIN_KEYS;

        // Copy the right half to new node
        for (int i = mid; i < full_child->key_count; i++) {
            new_node->keys[i - mid] = full_child->keys[i];
            if (full_child->is_leaf) {
                new_node->values[i - mid] = full_child->values[i];
            }
        }
        new_node->key_count = full_child->key_count - mid;
        full_child->key_count = mid;

        // Update next pointer for leaf nodes
        if (full_child->is_leaf) {
            new_node->next = full_child->next;
            full_child->next = new_node;
        } else {
            // Copy children for internal nodes
            for (int i = 0; i <= new_node->key_count; i++) {
                new_node->children[i] = full_child->children[i + mid];
            }
        }

        // Insert the new node into parent
        for (int i = parent->key_count; i >= index + 1; i--) {
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[index + 1] = new_node;

        for (int i = parent->key_count - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
        }
        parent->keys[index] = full_child->keys[mid];
        parent->key_count++;
    }

    // Insert into a non-full node
    void insertNonFull(Node* node, const std::string& key, int value) {
        int i = node->key_count - 1;

        if (node->is_leaf) {
            // Find position and check for duplicate value
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }

            // If key exists, check for duplicate value
            if (i >= 0 && node->keys[i] == key) {
                // Check if value already exists
                for (int j = 0; j < node->key_count; j++) {
                    if (node->keys[j] == key && node->values[j] == value) {
                        return;  // Duplicate key-value pair, don't insert
                    }
                }
            }

            // Shift to make room
            while (i >= 0 && node->keys[i] > key) {
                node->keys[i + 1] = node->keys[i];
                node->values[i + 1] = node->values[i];
                i--;
            }

            node->keys[i + 1] = key;
            node->values[i + 1] = value;
            node->key_count++;
        } else {
            // Find child to insert
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }
            i++;

            if (node->children[i]->key_count == MAX_KEYS) {
                splitChild(node, i, node->children[i]);
                if (node->keys[i] < key) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key, value);
        }
    }

    // Find the leaf node that should contain the key
    Node* findLeaf(Node* node, const std::string& key) {
        if (node->is_leaf) {
            return node;
        }

        int i = 0;
        while (i < node->key_count && node->keys[i] <= key) {
            i++;
        }
        return findLeaf(node->children[i], key);
    }

    // Remove a key from the tree
    void remove(Node* node, const std::string& key, int value) {
        if (node->is_leaf) {
            // Find and remove the key-value pair
            int idx = -1;
            for (int i = 0; i < node->key_count; i++) {
                if (node->keys[i] == key && node->values[i] == value) {
                    idx = i;
                    break;
                }
            }

            if (idx == -1) return;  // Key-value pair not found

            // Shift remaining elements
            for (int i = idx + 1; i < node->key_count; i++) {
                node->keys[i - 1] = node->keys[i];
                node->values[i - 1] = node->values[i];
            }
            node->key_count--;
        } else {
            // Find the child that should contain the key
            int i = 0;
            while (i < node->key_count && node->keys[i] <= key) {
                i++;
            }
            remove(node->children[i], key, value);
        }
    }

public:
    BPTree(const std::string& fname = "bptree.dat") : root(nullptr), filename(fname) {
        // Try to load from file
        std::ifstream file(filename, std::ios::binary);
        if (!file.good()) {
            // File doesn't exist, create new tree
            root = new Node(true);
        }
        file.close();
    }

    ~BPTree() {
        // Save tree to file if needed
        // For simplicity, we're not implementing persistent storage here
    }

    void insert(const std::string& key, int value) {
        if (root->key_count == MAX_KEYS) {
            Node* new_root = new Node(false);
            new_root->children[0] = root;
            splitChild(new_root, 0, root);

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
        if (root) {
            remove(root, key, value);
        }
    }

    std::vector<int> find(const std::string& key) {
        std::vector<int> result;

        if (!root) {
            return result;
        }

        Node* leaf = findLeaf(root, key);

        // Search in the leaf and linked leaves (for duplicates)
        Node* current = leaf;
        while (current) {
            for (int i = 0; i < current->key_count; i++) {
                if (current->keys[i] == key) {
                    result.push_back(current->values[i]);
                } else if (current->keys[i] > key) {
                    // Since keys are sorted, no more matches
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
    std::cin.ignore();

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
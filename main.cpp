#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

// Using a simple but efficient approach with STL
// Map key to a set of values (automatically sorted and unique)
class FileStorage {
private:
    std::map<std::string, std::set<int>> storage;

public:
    void insert(const std::string& key, int value) {
        storage[key].insert(value);
    }

    void remove(const std::string& key, int value) {
        auto it = storage.find(key);
        if (it != storage.end()) {
            it->second.erase(value);
            if (it->second.empty()) {
                storage.erase(it);
            }
        }
    }

    std::vector<int> find(const std::string& key) {
        std::vector<int> result;
        auto it = storage.find(key);
        if (it != storage.end()) {
            result.assign(it->second.begin(), it->second.end());
        }
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    FileStorage storage;

    int n;
    std::cin >> n;

    for (int i = 0; i < n; i++) {
        std::string command;
        std::cin >> command;

        if (command == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            storage.insert(key, value);
        } else if (command == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            storage.remove(key, value);
        } else if (command == "find") {
            std::string key;
            std::cin >> key;
            std::vector<int> values = storage.find(key);

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
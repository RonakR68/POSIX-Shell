#ifndef TRIE_H
#define TRIE_H

#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <dirent.h>
#include <cstring>

using namespace std;
namespace fs = std::filesystem;

class TrieNode {
public:
    map<char, TrieNode*> children;
    bool isEndOfWord;

    TrieNode() : isEndOfWord(false) {}
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
        init_trie(); // Initialize with commands from directories
    }

    void insert(const string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    void searchSuggestions(TrieNode* node, string prefix, vector<string>& suggestions) {
        if (node->isEndOfWord) {
            suggestions.push_back(prefix);
        }
        for (const auto& pair : node->children) {
            searchSuggestions(pair.second, prefix + pair.first, suggestions);
        }
    }

    vector<string> autocomplete(const string& prefix) {
        vector<string> suggestions;
        TrieNode* node = root;
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                return suggestions; // No suggestions
            }
            node = node->children[ch];
        }
        searchSuggestions(node, prefix, suggestions);
        return suggestions;
    }

    void init_trie() {
        const vector<string> directories = {"/bin", "/usr/bin", "/usr/local/bin", "/sbin"};

        for (const string& dir : directories) {
            DIR *directory = opendir(dir.c_str());
            if (directory) {
                struct dirent *entry;
                while ((entry = readdir(directory)) != NULL) {
                    if (entry->d_type == DT_REG || entry->d_type == DT_DIR) { // Regular file or directory
                        string name = entry->d_name;
                        if (!isContainsOtherThanCharacter(name.c_str())) {
                            insert(name); // Insert the command into the trie
                        }
                    }
                }
                closedir(directory);
            }
        }
    }

    bool isContainsOtherThanCharacter(const char *str) {
        for (int i = 0; str[i] != '\0'; i++) {
            if (!isalpha(str[i])) // Check for non-alphabetic characters
                return true;
        }
        return false;
    }

    vector<string> complete(const string& input) {
        return autocomplete(input);
    }
};

#endif // TRIE_H

//
// Created by lm on 2019/4/8.
//

#ifndef UPLOAD_SIMSEARCHER_TRIE_H
#define UPLOAD_SIMSEARCHER_TRIE_H

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <cstring>
#include <vector>
class TrieNode
{
public:
    TrieNode** child;
    //line numbers
    std::vector<int>* entries;
    //time(frequency) of their appearances
    std::vector<int>* entry_freq;
    int newest_line_num = -1;

    TrieNode(){
        child = new TrieNode*[128];
        memset(child,0,128*sizeof(TrieNode*));
        entries = new std::vector<int>();
        entry_freq = new std::vector<int>();
    }
    ~TrieNode(){
        delete[] child;
        delete entries;
        delete entry_freq;
    }
};
class Trie {
public:
    TrieNode* root;
    Trie() {root = new TrieNode();}
    void deleteNode(TrieNode* node){
        for (int i = 0; i < 128; i++) {
            if (node->child[i]) {
                deleteNode(node->child[i]);
            }
        }
        delete node;
    }

    ~Trie(){
        deleteNode(root);
    }
    void clean(){
    	deleteNode(root);
    	root = new TrieNode();
	}
    TrieNode* __insert_and_find(const char*, size_t);
    bool insert_multiple_unique(int num, const char *str, size_t len);
    int insert_single_line(int num, const char *str, size_t len);
    TrieNode* search(const char*, size_t);

};


#endif //UPLOAD_SIMSEARCHER_TRIE_H

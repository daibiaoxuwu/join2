//
// Created by lm on 2019/4/8.
//

#include <cstdio>
#include "Trie.h"

//for inserting lines by num in ascending order.
//for multiple lines. they share the same Trie and existance marked by vector.
//words in each line will be inserted uniquely.
//return true if it's not seen before and actually inserted.
bool Trie::insert_multiple_unique(int num, const char *str, size_t len) {
    TrieNode* node = __insert_and_find(str,len);

    //this word need to be unique in this line
    // if(node->newest_line_num < num){

    //     //if it's the first appearance of this word in this line
    //     //set newest_line_num to remember this appearance
    //     node->newest_line_num = num;

        //add this entry
        node->entries->push_back(num);
        // node->entry_freq->push_back(1);

    //     return true;
    // } else {
    //     *(node->entry_freq->end())++;
    //     return false;
    // }

    //this line has this word
}
bool Trie::addCount(const char *str, size_t len) {
    TrieNode* node = __insert_and_find(str,len);

    node->count++;
    return last_is_new;
}



//designed for a single line per Trie. existance marked by newest_line_num.
//if non-existant before, return -1. else return the num of the FIRST occurence.
int Trie::insert_single_line(int num, const char *str, size_t len) {
    TrieNode* node = __insert_and_find(str,len);

    int oldnum = node->newest_line_num;
    if(node->newest_line_num == -1) node->newest_line_num = num;
    return oldnum;
}

TrieNode* Trie::__insert_and_find(const char *str, size_t len) {
    TrieNode* node = root;
    for(int i = 0; i < (int)len; ++i){
        TrieNode* newNode = nullptr;
        for(auto nodepair : node->child){
            if(nodepair.first == (int)str[i]){
                newNode = nodepair.second;
                last_is_new = false;
                break;
            }
        }
        if(newNode == nullptr){
            last_is_new = true;
            newNode = new TrieNode();
            node->child.emplace_back(std::make_pair((int)str[i], newNode));
        }
        node = newNode;
    }
    return node;
//    auto search = node->entries.find(num);
//    printf("insert: %s %d count:%d\n",str, num, node->entries.count(num));

}


TrieNode* Trie::search(const char *str, size_t len) {
    TrieNode* node = root;
    for(int i = 0; i < (int)len; ++i){
        TrieNode* newNode = nullptr;
        for(auto nodepair : node->child){
            if(nodepair.first == (int)str[i]){
                newNode = nodepair.second;
                break;
            }
        }
        if(!newNode) {
            return nullptr;
        }
        node = newNode;
    }
    return node;
}


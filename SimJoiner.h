#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <functional>
#include "Trie.h"

using namespace std;

// struct TrieNode
// {
//     bool exist;
//     TrieNode *child[128];
//     std::vector<int> indexVec;
//     int count;
//     int id;

//     TrieNode() : exist(false), count(0), id(0)
//     {
//         for (int i = 0; i < 128; i++)
//             child[i] = NULL;
//         indexVec.clear();
//     }
// };

// struct Trie
// {
//     TrieNode* root;
//     bool empty;
//     int total;

//     Trie() {
//         total = 0;
//         root = new TrieNode();
//         empty = true;
//     }

//     void insert(const char* str, int len, int line) {
//         TrieNode *node = root;
//         for (int i = 0; i < len; i++) {
//             if (!node->child[(int)str[i]]) {
//                 node->child[(int)str[i]] = new TrieNode();
//             }
//             node = node->child[(int)str[i]];
//         }
//         node->exist = true;
//         if (node->indexVec.empty() || *(node->indexVec.end() - 1) != line)
//             node->indexVec.push_back(line);
//         empty = false;
//     }

//     std::vector<int>* search(const char* str, int len) {
//         TrieNode *node = root;
//         for (int i = 0; i < len; i++) {
//             if(!node->child[(int)str[i]])
//                 return NULL;
//             node = node->child[(int)str[i]];
//         }
//         if (!node)
//             return NULL;
//         if (!node->exist)
//             return NULL;
//         return &(node->indexVec);
//     }

//     int addCount(const char *str, int len) {
//         TrieNode *node = root;
//         for (int i = 0; i < len; i++) {
//             if (!node->child[(int)str[i]]) {
//                 node->child[(int)str[i]] = new TrieNode();
//             }
//             node = node->child[(int)str[i]];
//         }
//         int ret;
//         if (!node->exist)
//         {
//             total++;
//             ret = total;
//             node->id = total;
//             node->exist = true;
//         } else {
//             ret = node->id;
//         }
//         node->count++;
//         empty = false;
//         return ret;
//     }

//     int count(const char *str, int len) {
//         TrieNode *node = root;
//         for (int i = 0; i < len; i++) {
//             if(!node->child[(int)str[i]])
//                 return -1;
//             node = node->child[(int)str[i]];
//         }
//         if (!node)
//             return -1;
//         if (!node->exist)
//             return -1;
//         return node->count;
//     }

//     int getID(const char *str, int len) {
//         TrieNode *node = root;
//         for (int i = 0; i < len; i++) {
//             if(!node->child[(int)str[i]])
//                 return -1;
//             node = node->child[(int)str[i]];
//         }
//         if (!node)
//             return -1;
//         if (!node->exist)
//             return -1;
//         return node->id;
//     }

//     void init() {
//         total = 0;
//         root = new TrieNode();
//         empty = true;
//     }

//     bool isEmpty() {
//         return empty;
//     }

//     int getNum() {
//         return total;
//     }

//     void print() {
//         TrieNode *node = root;
//         for (int i = 0; i < 128; i++) {
//             if(!node->child[i])
//                 continue;
//             char a = i;
//             cout << a << endl;
//         }
//     }
// };


template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;

    bool operator < (const JoinResult &jr) const {
	    return id1 < jr.id1 || (id1 == jr.id1 && id2 < jr.id2);
	}
	bool operator == (const JoinResult &jr) const {
		return id1 == jr.id1 && id2 == jr.id2;
	}
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimJoiner {
public:

    bool isRead;
    // unordered_map<unsigned long long, vector<int> *> edMap[258][10];
    Trie edTrie[258][10];
    Trie jaccIDF;
    unordered_map<int, set<string>*> linewords[2];
    vector<int> **jaccList;

    vector<string> lines;
    vector<pair<string, int>> lines_short;
    int global_time;
    int *time_count;
    int createEDIndex(const char *filename, unsigned threshold);
    int createJaccIDF(const char *filename, int id);
    int createJaccIndex(const char *filename1, const char *filename2, double threshold);
    double compute_jaccard(std::set<string> *l1, std::set<string> *l2, double threshold);
    int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
    unsigned compute_ed(const char* s1, int len1, const char* s2, int len2, unsigned threshold);

    SimJoiner();
    ~SimJoiner();

    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);
};

#endif

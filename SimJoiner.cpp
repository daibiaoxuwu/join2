#include "SimJoiner.h"

#define HASH_SIZE 1000011
const int MY_MAX_INT = 0xffffff;

struct index_len
{
    int index;
    int len;

    bool operator< (const index_len& idl) const
    {
        return len < idl.len;
    }
};

inline bool comp(pair<int, int> a, pair<int, int> b)
{
    return a.second < b.second;
}

inline bool edcmp(const EDJoinResult& a, const EDJoinResult& b) {
    return ((a.id1 < b.id1) || (a.id1 == b.id1 && a.id2 < b.id2));
}

inline bool jaccCmp(const JaccardJoinResult& a, const JaccardJoinResult& b) {
    return ((a.id1 < b.id1) || (a.id1 == b.id1 && a.id2 < b.id2));
}

inline int my_abs(int a)
{
    if (a > 0)
        return a;
    else
        return -a;
}

inline int my_min(int a, int b)
{
    return a < b ? a : b;
}

inline int my_max(int a, int b)
{
    return a > b ? a : b;
}

inline int min_3(int x, int y, int z)
{
    return my_min(my_min(x, y), z);
}

inline int max_3(int x, int y, int z)
{
    return my_max(my_max(x, y), z);
}

inline unsigned long long my_hash(const char* s,int len)
{
    unsigned long long hash = 0;
    int seed = 131;
    for (int i = 0; i < len; i++) {
        hash = hash * seed + (int)s[i];
    }
    return hash & 0x7FFFFFFF;
}

SimJoiner::SimJoiner() {
    time_count = new int[300000]();
    global_time = 0;
    isRead = false;
    jaccList = new vector<int> *[1000000];
    for (int i = 0; i < 1000000; i++) {
        jaccList[i] = NULL;
    }
}

SimJoiner::~SimJoiner() {
}

int SimJoiner::createJaccIDF(const char *filename, int id) {
}

int SimJoiner::createJaccIndex(const char *filename1, const char *filename2, double threshold) {
}

double SimJoiner::compute_jaccard(set<string> *l1, set<string> *l2, double threshold)
{
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
  
    return SUCCESS;
}

int SimJoiner::createEDIndex(const char *filename, unsigned threshold) {
    lines.clear();
    lines_short.clear();

    char buf[1024];
    int line_count = 0;

    FILE* file = fopen(filename,"r");
	for(line_count=0;fgets(buf,1024,file);++line_count){

        int length = strlen(buf);
        if (length < threshold + 1) {
            lines_short.push_back(make_pair(buf, line_count));
            continue;
        }
        lines.push_back(buf);
        int step_d = length / (threshold + 1);
        int uptime = length - step_d * (threshold + 1);
        int step_u = uptime > 0 ? (step_d + 1) : step_d;
        int prefix_len = length - uptime * step_u;
        int i;
        for (i = 0; i * step_d < prefix_len; i++)
            edTrie[length][i].insert_multiple_unique(line_count, buf + i * step_d, step_d);
        for (int j = 0; j < uptime; j++)
            edTrie[length][i + j].insert_multiple_unique(line_count, buf + i * step_d + j * step_u, step_u);
    }
    isRead = true;
    return SUCCESS;
}

unsigned SimJoiner::compute_ed(const char* str1, int m, const char* str2, int n, unsigned threshold)
{
    if (my_abs(m - n) > threshold)
        return MY_MAX_INT;
    int dp[m+1][n+1];
        for (int i = 0; i <= my_min(threshold, m); i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= my_min(threshold, n); j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= m; i++)
    {
        int begin = my_max(i - threshold, 1);
        int end = my_min(i + threshold, n);
        if (begin > end)
            break;
        for (int j = begin; j <= end; j++)
        {
            int t = !(str1[i - 1] == str2[j - 1]);
            int d1 = my_abs(i - 1 - j) > threshold ? MY_MAX_INT : dp[i - 1][j];
            int d2 = my_abs(i - j + 1) > threshold ? MY_MAX_INT : dp[i][j - 1];
            dp[i][j] = min_3(
                d1 + 1,
                d2 + 1,
                dp[i - 1][j - 1] + t);
        }
    }
    return dp[m][n];
}

int SimJoiner::searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result) {
    global_time++;
    result.clear();
    vector<int> cand_lines;
    int lineLength = strlen(query);
    int upperbound = my_min(lineLength + threshold, 256);
    int lowerbound = my_max(0, lineLength - threshold);
    for (int length = lowerbound; length <= upperbound; length++)
    {
        for(int it = 0; it < 128; ++it) if (edTrie[length][0].root->child[it]!=nullptr){
        int step_d = length / (threshold + 1);
        int uptime = length - step_d * (threshold + 1);
        int step_u = uptime > 0 ? (step_d + 1) : step_d;
        int prefix_len = length - uptime * step_u;
        int delta = lineLength - length;
        int i;
        for (i = 0; i * step_d < prefix_len; i++)
        {
            int p = i * step_d;
            int start = max_3(p - i, p + delta - ((int)threshold - i), 0);
            int end = min_3(p + i, p + delta + ((int)threshold - i), lineLength - step_d);
            for (int strid = start; strid <= end; strid++) {
                TrieNode*  node = edTrie[length][i].search(query + strid, step_d);
                if (node) {
                    for (auto& candi : *(node->entries)) {
                        if (time_count[candi] != global_time) {
                            time_count[candi] = global_time;
                            unsigned ed = compute_ed(query, lineLength, lines[candi].c_str(), lines[candi].length(), threshold);
                            if (ed <= threshold)
                            {
                                result.push_back(make_pair(candi, ed));
                            }
                        }
                    }
                }
            }
        }
        for (int j = 0; j < uptime; j++)
        {
            int p = i * step_d + j * step_u;
            int start = max_3(p - i - j, p + delta - ((int)threshold - i - j), 0);
            int end = min_3(p + i + j, p + delta + ((int)threshold - i - j), lineLength - step_u);
            for (int strid = start; strid <= end; strid++)
            {
                TrieNode*  node =edTrie[length][i + j].search(query + strid, step_u);
                if (node) {
                    for (auto& candi : *(node->entries))
                    {
                        if (time_count[candi] != global_time) {
                            time_count[candi] = global_time;
                            unsigned ed = compute_ed(query, lineLength, lines[candi].c_str(), lines[candi].length(), threshold);
                            if (ed <= threshold) {
                                result.push_back(make_pair(candi, ed));
                            }
                        }
                    }
                }
            }
        }
        }
    }
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();

    for(int i = 0; i < 258; ++i){
        for(int j=0; j<10; ++j){
            edTrie[i][j].clean();
        }
    }




    createEDIndex(filename2, threshold);
    ifstream fin(filename1);
    string str;
    vector<pair<unsigned, unsigned> > resultED;
    int id1 = 0;
    while (getline(fin, str))
    {
        searchED(str.c_str(), threshold, resultED);
        for (auto &it : resultED)
        {
    		EDJoinResult temp;
    		temp.id1 = id1;
    		temp.id2 = it.first;
    		temp.s = it.second;
    		result.push_back(temp);
    	}
        for (auto& lines : lines_short) {
            string &shortstr = lines.first;
            int ed;
            if ((ed = compute_ed(str.c_str(), str.length(), shortstr.c_str(), shortstr.length(), threshold)) <= threshold)
            {
                EDJoinResult temp;
                temp.id1 = id1;
                temp.id2 = lines.second;
                temp.s = ed;
                result.push_back(temp);
            }
        }
    	id1++;
    }
    fin.close();
    sort(result.begin(), result.end());
    return SUCCESS;
}

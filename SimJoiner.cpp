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
    char buf[1024];
	FILE* file = fopen(filename,"r");
	for(int line_count=0;fgets(buf,1024,file);++line_count){
        string str1 = string(buf);
        set<string> *tmp = new set<string>();
        filetext[id].push_back(str1);
        int len = str1.length();
        int pos = 0;
        for (int j = 0; j < len; j++)
        {
            if(str1[j] == ' ')
            {
                if(pos < j)
                {
                    jaccIDF.addCount(str1.substr(pos, j - pos).c_str(), j - pos);
                    tmp->insert(str1.substr(pos, j - pos));
                }
                pos = j + 1;
            }
        }
        if (pos < len)
        {
            jaccIDF.addCount(str1.substr(pos, len - pos).c_str(), len - pos);
            tmp->insert(str1.substr(pos, len - pos));
        }
        linewords[id][line_count] = tmp;
        line_count++;
    }
    fin1.close();
    return SUCCESS;
}

int SimJoiner::createJaccIndex(const char *filename1, const char *filename2, double threshold) {
    createJaccIDF(filename1, 0);
    createJaccIDF(filename2, 1);
    int totalNum = linewords[1].size();
    for (int i = 0; i < totalNum; i++)
    {
        vector<index_len> vec_index;
        for (auto &s : *(linewords[1][i])) {
            index_len idl;
            idl.len = jaccIDF.count(s.c_str(), s.length());
            idl.index = jaccIDF.getID(s.c_str(), s.length());
            vec_index.push_back(idl);
        }
        sort(vec_index.begin(), vec_index.end());
        int prelen = (1 - threshold) * linewords[1][i]->size() + 1;
        for (int j = 0; j < prelen; j++)
        {
            index_len &idl = vec_index[j];
            if (!jaccList[idl.index]) {
                vector<int>* listtmp = new vector<int>();
                listtmp->push_back(i);
                jaccList[idl.index] = listtmp;
            } else {
                jaccList[idl.index]->push_back(i);
            }
        }
    }

    return SUCCESS;
}

double SimJoiner::compute_jaccard(set<string> *l1, set<string> *l2, double threshold)
{
    int cnt = 0;
    for (auto& w : *l1)
    {
        if (l2->find(w) != l2->end())
            cnt++;
    }
    return ((double)cnt / (double)(l1->size() + l2->size() - cnt));
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    createJaccIndex(filename1, filename2, threshold);
    int totalNum = linewords[0].size();
    for (int i = 0; i < totalNum; i++) {
        vector<index_len> vec_index;
        for (auto &s : *(linewords[0][i])) {
            index_len idl;
            idl.len = jaccIDF.count(s.c_str(), s.length());
            idl.index = jaccIDF.getID(s.c_str(), s.length());
            vec_index.push_back(idl);
        }
        sort(vec_index.begin(), vec_index.end());
        int prelen = (1 - threshold) * linewords[0][i]->size() + 1;
        for (int j = 0; j < prelen; j++)
        {
            index_len &idl = vec_index[j];
            if (jaccList[idl.index]) {
                for (auto& lineid : *jaccList[idl.index]) {
                    double jacc = compute_jaccard(linewords[0][i], linewords[1][lineid], threshold);
                    if (jacc >= threshold) {
                        JaccardJoinResult temp;
                        temp.id1 = i;
                        temp.id2 = lineid;
                        temp.s = jacc;
                        result.push_back(temp);
                    }
                }
            }
        }
    }
    sort(result.begin(), result.end());
    result.resize(unique(result.begin(), result.end()) - result.begin());
    return SUCCESS;
}

int SimJoiner::createEDIndex(const char *filename, unsigned threshold) {
    contexts.clear();
    EDshort.clear();
    ifstream fin(filename);
    char linechar[260];
    int line_count = 0;
    while (fin.getline(linechar, 260))
    {
        int length = strlen(linechar);
        if (length < threshold + 1) {
            EDshort.push_back(make_pair(linechar, line_count));
            line_count++;
            continue;
        }
        contexts.push_back(linechar);
        int step_d = length / (threshold + 1);
        int uptime = length - step_d * (threshold + 1);
        int step_u = uptime > 0 ? (step_d + 1) : step_d;
        int prefix_len = length - uptime * step_u;
        int i;
        for (i = 0; i * step_d < prefix_len; i++)
        {
            unsigned long long h = my_hash(linechar + i * step_d, step_d);
            auto it = edMap[length][i].find(h);
            if (it == edMap[length][i].end()) {
                vector<int> *tempVec = new vector<int>();
                tempVec->push_back(line_count);
                edMap[length][i][h] = tempVec;
            } else {
                it->second->push_back(line_count);
            }
        }
        for (int j = 0; j < uptime; j++)
        {
            unsigned long long h = my_hash(linechar + i * step_d + j * step_u, step_u);
            auto it = edMap[length][i + j].find(h);
            if (it == edMap[length][i + j].end()) {
                vector<int> *tempVec = new vector<int>();
                tempVec->push_back(line_count);
                edMap[length][i + j][h] = tempVec;
            } else {
                it->second->push_back(line_count);
            }
        }
        line_count++;
    }
    fin.close();
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
        if (edMap[length][0].empty())
            continue;
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
                unsigned long long hash = my_hash(query + strid, step_d);
                vector<int> *listptr = edMap[length][i][hash];
                if (listptr) {
                    for (auto& candi : *listptr) {
                        if (time_count[candi] != global_time) {
                            time_count[candi] = global_time;
                            unsigned ed = compute_ed(query, lineLength, contexts[candi].c_str(), contexts[candi].length(), threshold);
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
                unsigned long long hash = my_hash(query + strid, step_u);
                vector<int> *listptr = edMap[length][i + j][hash];
                if (listptr)
                {
                    for (auto &candi : *listptr)
                    {
                        if (time_count[candi] != global_time) {
                            time_count[candi] = global_time;
                            unsigned ed = compute_ed(query, lineLength, contexts[candi].c_str(), contexts[candi].length(), threshold);
                            if (ed <= threshold) {
                                result.push_back(make_pair(candi, ed));
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
        for (auto& edshort : EDshort) {
            string &shortstr = edshort.first;
            int ed;
            if ((ed = compute_ed(str.c_str(), str.length(), shortstr.c_str(), shortstr.length(), threshold)) <= threshold)
            {
                EDJoinResult temp;
                temp.id1 = id1;
                temp.id2 = edshort.second;
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

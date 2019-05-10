#include "SimJoiner.h"

#define HASH_SIZE 1000011
const int MY_MAX_INT = 0xffffff;

struct index_len
{
    TrieNode* index;
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


inline JaccardJoinResult create_JaccardJoinResult(unsigned id1, unsigned id2, double s){
    JaccardJoinResult result;
    result.id1 = id1;
    result.id2 = id2;
    result.s = s;
    return result;
}
inline EDJoinResult create_EDJoinResult(unsigned id1, unsigned id2, unsigned s){
    EDJoinResult result;
    result.id1 = id1;
    result.id2 = id2;
    result.s = s;
    return result;
}


SimJoiner::SimJoiner() {
    aval_list = new bool[200010];
}

SimJoiner::~SimJoiner() {
}

int SimJoiner::createJaccIDF(const char *filename, int id) {
    char buf[1024];
	FILE* file = fopen(filename,"r");
	for(int line_count=0;fgets(buf,1024,file);++line_count){
        if(buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]='\0';
        string str1 = string(buf);
        set<string> *tmp = new set<string>();

	    char* pch = strtok (buf," \r\n");
		while (pch != nullptr)
		{
			//if is unique
            jaccIDF.addCount(str1.substr(pos, j - pos).c_str(), j - pos);
			pch = strtok (nullptr, " \r\n");
		}
	
    }
    fclose(file);
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
            TrieNode* node = jaccIDF.search(s.c_str(), s.length());
            if(node){
                idl.len = node->count;
                idl.index = node;
                vec_index.push_back(idl);
            }
        }
        //在文件1的 第i行里选取全局最高频率那1-thresh个
        //到排表: 词汇名->行数
        sort(vec_index.begin(), vec_index.end());
        int prelen = (1 - threshold) * linewords[1][i]->size() + 1;
        for (int j = 0; j < my_min(prelen, vec_index.size()); j++)
            vec_index[j].index->entries->push_back(i);
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
    jaccIDF.clean();
    createJaccIndex(filename1, filename2, threshold);

    memset(aval_list, 0, 200010*sizeof(bool));


    int totalNum = linewords[0].size();
    for (int i = 0; i < totalNum; i++) {
        vector<index_len> vec_index;
        for (auto &s : *(linewords[0][i])) {
            index_len idl;
            TrieNode* node = jaccIDF.search(s.c_str(), s.length());
            if(node){
                idl.len = node->count;
                idl.index = node;
                vec_index.push_back(idl);
            }
        }
        //在文件0的 第i行里选取全局最高频率那1-thresh个
        //到排表: 词汇名->行数
        sort(vec_index.begin(), vec_index.end());
        int prelen = (1 - threshold) * linewords[0][i]->size() + 1;
        for (int j = 0; j < my_min(prelen, vec_index.size()); j++)
        {
            for (auto& lineid : *(vec_index[j].index->entries)){
                if(aval_list[lineid]) continue;
                aval_list[lineid] = true;
                double jacc = compute_jaccard(linewords[0][i], linewords[1][lineid], threshold);
                if (jacc >= threshold) 
                    result.push_back(create_JaccardJoinResult(i, lineid, jacc));
            }
        }
    }
    sort(result.begin(), result.end());
    return SUCCESS;
}


int SimJoiner::createEDIndex(const char *filename, unsigned threshold) {
    lines.clear();
    lines_short.clear();

    char buf[1024];
    int line_count = 0;

    FILE* file = fopen(filename,"r");
	for(line_count=0;fgets(buf,1024,file);++line_count){
        if(buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]='\0';
        int length = strlen(buf);
        lines.push_back(string(buf));
        // if (length < threshold + 1) {
        //     lines_short.push_back(make_pair(string(buf), line_count));
        //     continue;
        // }
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
    return SUCCESS;
}


unsigned SimJoiner::calculate_ED(const char *query, const char *line, unsigned threshold){
	const int query_len = (int)strlen(query);
	const int line_len = (int)strlen(line);
	unsigned data[query_len+1][line_len+1];

	//initialize
	for(unsigned i = 0; i <= query_len; ++i) data[i][0]=i;
	for(unsigned i = 0; i <= line_len; ++i) data[0][i]=i;
	//end answer
	data[query_len][line_len] = threshold + 1;

	int left = 1, right = my_min(line_len,1+threshold);
	for(int i = 1; i <= query_len; ++ i){
		if(left>right) return threshold + 1;

		//narrow range for next round
		bool leftflag = true;
		int next_right = left;

		left=my_max(left,i-threshold);
		//leftmax for next round
		if(left>1) data[i][left-1]=threshold+1;

		for(int j = left; j <= right; ++ j){
			data[i][j] = min_3(data[i-1][j-1] + (query[i-1] != line[j-1]),data[i-1][j] + 1, data[i][j-1]+1);

			//narrow the left and right for next iter
			if(data[i][j]>threshold){
				//find the leftmost "<threshold"
				if(leftflag) left = j;
			} else {
				leftflag = false;
				//find the rightmost "<threshold"
				next_right = j;
			}
		}
		//cannot reach the end, terminate
		if(next_right + query_len - i < line_len) return threshold +1;

		if(next_right<line_len) {
			right = next_right + 1;
			//rightmax for next round
			data[i][right] = threshold + 1;
		} else{
			right =next_right;
		}
	}
	return data[query_len][line_len];

}

int SimJoiner::searchED(const char *query, int id1, unsigned threshold,  vector<EDJoinResult> &result){

    memset(aval_list, 0, 200010*sizeof(bool));

    int lineLength = strlen(query);
    int upperbound = my_min(lineLength + threshold, 256);
    int lowerbound = my_max(0, lineLength - threshold);
    for (int length = lowerbound; length <= upperbound; length++)
    {
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
                    for (auto& candi : *(node->entries))
                        aval_list[candi] = true;
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
                        aval_list[candi] = true;
                }
            }
        }
        for(int i = 0; i < 200010; ++i){
            if(aval_list[i]){
                unsigned ed_value = calculate_ED(query, lines[i].c_str(), threshold);
                if (ed_value <= threshold)
                    result.push_back(create_EDJoinResult(id1,i,ed_value));
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
        char* buf = new char[258];
		memcpy(buf, str.c_str(), 258);
        if(buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]='\0';
        searchED(buf, id1, threshold, result);
        
        // for (auto& lines : lines_short) {
        //     string &shortstr = lines.first;
        //     int ed;
        //     if ((ed = compute_ed(str.c_str(), str.length(), shortstr.c_str(), shortstr.length(), threshold)) <= threshold)
        //     {
        //         EDJoinResult temp;
        //         temp.id1 = id1;
        //         temp.id2 = lines.second;
        //         temp.s = ed;
        //         result.push_back(temp);
        //     }
        // }
    	id1++;
    }
    fin.close();
    sort(result.begin(), result.end());
    result.resize(unique(result.begin(), result.end()) - result.begin());
    return SUCCESS;
}

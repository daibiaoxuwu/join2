#include "SimSearcher.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace std;

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

SimSearcher::SimSearcher()
{
	word_count = new int[200010];
	char_count = new int[200010];
	lines = new char*[200010];
	same = new int[200010];
	memset(word_count, 0, 200010*sizeof(int));
	memset(char_count, 0, 200010*sizeof(int));
	qgrams = new char*[500];
	qgrams_freq = new int[500];
	memset(qgrams_freq,0,500* sizeof(int));
}

SimSearcher::~SimSearcher()
{
	delete[](word_count);
	delete[](char_count);
	delete[](same);
	for(int i = 0; i < line_count && lines[i]!= nullptr; ++ i){
		delete[](lines[i]);
	}
	delete[](lines);
	delete[] qgrams_freq;
	delete[] qgrams;
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
	char buf[1024];
	FILE* file = fopen(filename,"r");
	for(line_count=0;fgets(buf,1024,file);++line_count){

//	    if(line_count%10000==0) printf("read:%d\n",line_count);//checkpoint

		if(buf[strlen(buf)-1]=='\n'){
			buf[strlen(buf)-1]='\0';
		}
//		printf("str:%s\n",buf);
		char_count[line_count] = (int)strlen(buf);
		lines[line_count] = new char[strlen(buf)+1];
		memcpy(lines[line_count],buf,strlen(buf)+1);



		//ED: insert adjacent q words into edTrie
		this->q=q;
		for(int j = 0; buf[j+q-1]!='\0'; ++j){
			edTrie.insert_multiple_unique(line_count, buf + j, q);
		}


		//printf("%s count:%d copied to %s\n",buf,strlen(buf),lines[line_count]);

		//jaccard: split by spaces and insert into jacTrie
		char* pch = strtok (buf," \r\n");
		while (pch != nullptr)
		{
		   	if(jacTrie.insert_multiple_unique(line_count, pch, strlen(pch)))
		   		++word_count[line_count];
			pch = strtok (nullptr, " \r\n");
		}



        /*
		std::unordered_set<int> aset = *jacTrie.search("line",4);
		for (auto it=aset.begin(); it!=aset.end(); it=aset.upper_bound(*it)){
			printf("%d ", *it);
		}
        for(int i2 = 0;i2 < 5; ++ i2){
            printf("find:%d\n",aset.count(i2));
		}
        */


	}



	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();


	//copy query into query_split to avoid modifying it in 'strtok'
	char* query_split = new char[strlen(query)+1];
	memcpy(query_split,query,strlen(query)+1);
	int query_count = 0;

	//split query into words; then find them in jacTrie
	//search function accepts a word and returns a small hash_set
	//containing ids of lines that owns this word.
	//to count multiple words, arrange their pointer in multisets.
	//then count each pointer.
	memset(same, 0, 200010*sizeof(int));

	//count multiple words
	Trie query_trie;
	char* pch = strtok(query_split," \r\n");
	while (pch != nullptr)
	{
		//if this word is unique in query
		if(query_trie.insert_single_line(0, pch, strlen(pch)) == -1) {
			//count number of words
			++query_count;
			//search for this word in jacTrie
			TrieNode* search_result = jacTrie.search(pch, strlen(pch));
			if(search_result != nullptr){
				for (int it2 : *(search_result->entries)) ++same[it2];
			}
		}
		pch = strtok (nullptr, " \r\n");
	}


//	printf("jac parsed entry. wordsets:%d\n",word_sets.size());//checkpoint


	//find differences between numbers of a word in query and in entry.
	//iterate through each word common in query and jacTrie

	//diff of all lines

//	int count = 0;//checkpoint


//	count = 0;//checkpoint


	for(int i = 0; i < line_count; ++i){
//		if(i%10000==0) printf("calc jac:%d\n",i);//checkpoint

//		if(++count % 1000 == 0) printf("calc jac processed:%d\n",count);//checkpoint

		double jaccard = same[i] / (double)(query_count + word_count[i] - same[i]);
		if(jaccard>=threshold)
            result.emplace_back(make_pair(i, jaccard));
	}
	delete[](query_split);

	return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{

	result.clear();
	int query_count = (int)strlen(query);

//	if(query_count - q + 1 <= threshold){
//		for(int i = 0; i < line_count; ++ i){
//			if(my_abs(query_count-char_count[i])>threshold) continue;
//			int ed_value = calculate_ED(query, lines[i], (int) threshold);
//			if (ed_value <= threshold)
//				result.emplace_back(make_pair(i, ed_value));
//		}
//		return SUCCESS;
//	}


	//----start

	//split query into words; then find them in jacTrie
	//search function accepts a word and returns a small hash_set
	//containing ids of lines that owns this word.
	//to count multiple words, arrange their pointer in sets.
	//then count each pointer.
	Trie query_trie;




	//the num of unique phrases in query. used only in below to count phrases.
	int phrase_num = 0;

	char* query_split = new char[strlen(query)+1];
	memcpy(query_split,query,strlen(query)+1);

	for(char* pch = query_split; pch[q-1] != '\0'; ++pch) {
		//for the string pch, find if it's the first one in query.
		//if not, find its first occurance.
		int first_occurance = query_trie.insert_single_line(phrase_num, pch, strlen(pch));

		//a new string
		if (first_occurance == -1) {
			//i is its first occurance and has been written inside Trie.
			//when the same string appears again, first_occurance will be i instead of -1.
			//at the third, fourth,... appearance first_occurance will always remain i.
			qgrams_freq[phrase_num] = 1;
			qgrams[phrase_num] = pch;
			++phrase_num;
		} else {
			//it has appeared. count its time to query_num.
			++qgrams_freq[first_occurance];
		}
	}
	memset(same, 0, 200010*sizeof(int));

	//for each phrase:
	for(int i = 0; i < phrase_num; ++i) {

		//search for this word in jacTrie
		TrieNode *search_result = edTrie.search(qgrams[i], q);
		if (search_result != nullptr) {
			//double vector of a Trienode.
			//the lines containing this phrase
			auto line_num_it = search_result->entries->begin();
			for (int entry_num : (*search_result->entry_freq)) {
				same[*line_num_it] += my_min(qgrams_freq[i], entry_num);
				++line_num_it;
			}
		}
	}


	for(int i =0;i<line_count;++i){

		//too long or too short
		if(my_abs(query_count-char_count[i])>threshold) continue;


		int diff_edit_movesxq = my_abs(query_count - char_count[i]) * q;
		int change_edit_movesxq = (my_max(query_count, char_count[i]) - q + 1 - same[i]);
		int ed_maxxq = my_max(diff_edit_movesxq , change_edit_movesxq);

//		int ed_value = calculate_ED(query, lines[i], (int) threshold);
//		if(ed_maxxq > ed_value*q) printf("error! %d %d\n",ed_maxxq, ed_value);

		if(ed_maxxq <= threshold * q){
//        if(true){
//			if(count++%1000==0)printf("ed:%d\n",count);//checkpoint
			int ed_value = calculate_ED(query, lines[i], (int) threshold);
			if (ed_value <= threshold)
				result.emplace_back(make_pair(i, ed_value));
		}
	}

    delete[] query_split;
	return SUCCESS;
}




int SimSearcher::calculate_ED(const char *query, char *line, int threshold){
//    printf("%s %s\n",query,line);
	const int query_len = (int)strlen(query);
	const int line_len = (int)strlen(line);
	int data[query_len+1][line_len+1];
//	for(int i = 0; i <= query_len; ++ i)
//		for(int j = 0; j <= line_len; ++ j)
//			data[i][j]=-1;
	for(int i = 0; i <= query_len; ++i) data[i][0]=i;
	for(int i = 0; i <= line_len; ++i) data[0][i]=i;
	//endmax
	data[query_len][line_len] = threshold + 1;


	int left = 1, right = my_min(line_len,1+threshold);
	for(int i = 1; i <= query_len; ++ i){
//		printf("%d %d\n",left,right);
//		for(int i = 0; i <= query_len; ++ i, printf("\n"))
//			for(int j = 0; j <= line_len; ++ j)
//				printf("%d\t",data[i][j]);
		//not found
		if(left>right) return threshold + 1;

		//left my_max
//		data[i][left-1] = threshold + 1;

		//narrow range for next round
		bool leftflag = true;
		int next_right = left;

		left=my_max(left,i-threshold);
		if(left>1) data[i][left-1]=threshold+1;

//		if(i-threshold>1) data[i][i-threshold-1]=threshold+1;
		for(int j = left; j <= right; ++ j){

//			if(data[i-1][j-1]==-1 || data[i][j-1]==-1 || data[i-1][j]==-1){
//				printf("error!!!");
//				return threshold +1;
//			}

			data[i][j] = min_3(data[i-1][j-1] + (query[i-1] != line[j-1]),data[i-1][j] + 1, data[i][j-1]+1);
			//narrow the left and right for next iter
			if(data[i][j]>threshold){
				if(leftflag) left = j;
			} else {
				leftflag = false;
				next_right = j;
			}
		}
		if(next_right + query_len - i< line_len) return threshold +1;
		if(next_right<line_len) {
			right = next_right + 1;
			data[i][right] = threshold + 1;
		} else{
			right =next_right;
		}
//		if(right!= line_len) data[i][right] = threshold + 1;

	}
//	for(int i = 0; i <= query_len; ++ i, printf("\n"))
//		for(int j = 0; j <= line_len; ++ j)
//			printf("%d\t",data[i][j]);
	return data[query_len][line_len];

}

#include <vector>
#include <utility>
#include "Trie.h"
const int SUCCESS = 0;
const int FAILURE = 1;

class SimSearcher
{
	Trie jacTrie, edTrie;
	//count number of word in each line
	int* word_count;
	int* char_count;
	char** lines;

	int* same;

	char** qgrams;
	int* qgrams_freq;

	//count number of lines
	int line_count;
	int q;

	int calculate_ED(const char *query, char* line, int threshold);
public:
	SimSearcher();
	~SimSearcher();

	int createIndex(const char *filename, unsigned q);
	int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double> > &result);
	int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned> > &result);
};


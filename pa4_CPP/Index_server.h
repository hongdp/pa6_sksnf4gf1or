#ifndef INDEX_SERVER_H
#define INDEX_SERVER_H

#include <iosfwd>
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
#include <map>

struct Query_hit {
	Query_hit(const std::string& id_, double score_)
	: id(id_), score(score_), times(1)
	{}
	
	std::string id;
	double score;
	int times;
	bool operator< (const Query_hit& rhs) const{
		return id < rhs.id;
	}
};

class Index_server {
public:
	void run(int port);
	
	// Methods that students must implement.
	void init(std::ifstream& infile);
	void process_query(const std::string& query, double weight, std::vector<Query_hit>& hits);
private:
	struct weight{
		int doc_id;
		double weight;
	};
	
	struct word_info{
		double idf;
		std::vector <weight> weights;
	};
	std::map<std::string,word_info> index_map;
	std::set<std::string> stop_words;
	std::map<int, double> pr_map;
};

#endif
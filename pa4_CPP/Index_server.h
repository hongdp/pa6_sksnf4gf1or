#ifndef INDEX_SERVER_H
#define INDEX_SERVER_H

#include <iosfwd>
#include <stdint.h>
#include <string>
#include <vector>

struct Query_hit {
	Query_hit(const std::string& id_, double score_)
        : id(id_), score(score_)
        {}

	std::string id;
    double score;
	bool operator< (const Query_hit& rhs) const{
		return id < rhs.id;
	}
};

class Index_server {
public:
    void run(int port);

    // Methods that students must implement.
    void init(std::ifstream& infile);
    void process_query(const std::string& query, std::vector<Query_hit>& hits);
};

#endif

#ifndef INDEXER_H
#define INDEXER_H

#include <iosfwd>
#include <set>
#include <map>
#include <string>

class Indexer {
public:
	// Load stop words from file;
	Indexer();
	// Construct inverted index from content;
    void index(std::ifstream& content, std::ostream& outfile);
private:
	std::set<std::string> stop_words;
};

#endif

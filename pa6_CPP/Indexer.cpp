#include "Indexer.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <cmath>
#include <functional>
#include <string>
using namespace std;


// Reads content from the supplied input file stream, and transforms the
// content into the actual on-disk inverted index file.

struct Doc_Info_T {
	int doc_id;
	int num_occurance;
};

void Indexer::index(ifstream& content, ostream& outfile)
{
    // Fill in this method to parse the content and build your
    // inverted index file.
	int total_doc_num = 0;
	map<string, vector<Doc_Info_T> > word_infos;
	map<string, double> idfs;
	map<int, double> normalization_factors;
	string caption;
	while (getline(content, caption)) {
		map<string, int> word_occurance;
		stringstream strstream(caption);
		string word;
		total_doc_num++;
		while (strstream >> word) {
			for_each(word.begin(), word.end(), [](char& ch){ch = tolower(ch);});
			if (stop_words.find(word) != stop_words.end()) {
				continue;
			}
			try {
				for(auto it = word.begin(); it != word.end();){
					if (!isalnum(*it)) {
						word.erase(it);
					} else {
						it++;
					}
				}
			} catch (...) {
				cout << word << endl;
			}
			
			if (word.empty()) {
				continue;
			}
			word_occurance[word]++;
		}
		for (auto it = word_occurance.begin(); it != word_occurance.end(); it++) {
			word_infos[it->first].push_back(Doc_Info_T{total_doc_num, it->second});
		}
	}
	
	for_each(word_infos.begin(), word_infos.end(), [&idfs, &normalization_factors, &total_doc_num](const pair<string, vector<Doc_Info_T> >& doc_info_pair){
		double idf = log10(((double)total_doc_num)/doc_info_pair.second.size());
		idfs[doc_info_pair.first] = idf;
		for_each(doc_info_pair.second.begin(), doc_info_pair.second.end(), [idf, &normalization_factors](const Doc_Info_T& doc_info){
			normalization_factors[doc_info.doc_id] += pow(doc_info.num_occurance*idf, 2);
		});
	});
	
	for (auto it = word_infos.begin(); it != word_infos.end(); it++) {
		int total_occurance = 0;
		for_each(it->second.begin(), it->second.end(), [&total_occurance](const Doc_Info_T& doc_info){
			total_occurance += doc_info.num_occurance;
		});
		double idf = idfs[it->first];
		outfile << it->first << " " << idf << " " << total_occurance;
		for_each(it->second.begin(), it->second.end(), [&outfile, &normalization_factors](const Doc_Info_T& doc_info){
			outfile << " " << doc_info.doc_id << " " << doc_info.num_occurance << " " << normalization_factors[doc_info.doc_id];
		});
		outfile << endl;
	}

}

Indexer::Indexer()
{
    ifstream myFile;
    myFile.open("stop_words.txt");
    std::string word;
    if (myFile.is_open()) {
      while (myFile >> word) {
        //add word to set
        stop_words.insert(word);
        //cout << word<< endl;
      }
    }
    myFile.close();
}

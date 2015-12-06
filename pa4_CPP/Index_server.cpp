#include "Index_server.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>

#include "mongoose.h"

using std::string;
using std::vector;
using std::ostream;
using std::map;
using std::stringstream;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::ifstream;
using std::cout;

namespace {
	int handle_request(mg_connection *);
	int get_param(const mg_request_info *, const char *, string&);
	string get_param(const mg_request_info *, const char *);
	string to_json(const vector<Query_hit>&);
	
	ostream& operator<< (ostream&, const Query_hit&);
}


pthread_mutex_t mutex;

// Runs the index server on the supplied port number.
void Index_server::run(int port)
{
	// List of options. Last element must be NULL
	ostringstream port_os;
	port_os << port;
	string ps = port_os.str();
	const char *options[] = {"listening_ports",ps.c_str(),0};
	
	// Prepare callback structure. We have only one callback, the rest are NULL.
	mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = handle_request;
	
	// Initialize the global mutex lock that effectively makes this server
	// single-threaded.
	pthread_mutex_init(&mutex, 0);
	
	// Start the web server
	mg_context *ctx = mg_start(&callbacks, this, options);
	if (!ctx) {
		cerr << "Error starting server." << endl;
		return;
	}
	
	pthread_exit(0);
}


// Load index data from the file of the given name.
void Index_server::init(ifstream& infile)
{
	string index;
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
	ifstream num_of_docs_file("num_of_docs.txt");
	int num_of_docs;
	num_of_docs_file >> num_of_docs;
	num_of_docs_file.close();
	int counter = 0;
	while(getline(infile, index)){
		if (!(counter % 10000)) {
			cout << "No." << counter << endl;
			cout << "Index: " << index << endl;
		}
		counter++;
		stringstream ss;
		ss<<index;
		string word;
		ss>>word;
		
		int df;
		ss>>df;
		vector<weight> ws;
		double idf = log10(num_of_docs/(double)df);
		for(int i = 0; i<df; i++){
			int doc_id;
			ss>>doc_id;
			char seperator;
			ss >> seperator;
			double normed_tf_idf;
			ss>>normed_tf_idf;
			
			weight we;
			we.doc_id = doc_id;
			we.weight = normed_tf_idf;
			
			ws.push_back(we);
		}
		
		word_info w_i;
		w_i.idf = idf;
		w_i.weights = ws;
		
		index_map[word] = w_i;
	}
	
	ifstream page_rank_file("PageRank.txt");
	if (page_rank_file) {
		string line;
		int pageRankCounter = 0;
		while (getline(page_rank_file, line)) {
			if (!(pageRankCounter % 1000)) {
				cout << "No." << pageRankCounter << endl;
				cout << "Index: " << line << endl;
			}
			stringstream sstream(line);
			int doc_id;
			double page_rank_value;
			sstream >> doc_id >> page_rank_value;
			pr_map[doc_id] = page_rank_value;
		}
	}
	cout << "Init Finished" << endl;
	// Fill in this method to load the inverted index from disk.
	
}

// Search the index for documents matching the query. The results are to be
// placed in the supplied "hits" vector, which is guaranteed to be empty when
// this method is called.
void Index_server::process_query(const string& query, double weight, vector<Query_hit>& hits)
{
	struct token_info_t{
		int feq;
		double w;
	};
	cout << "Processing query '" << query << "'" << endl;
	double normalization_factor = 0;
	// Fill this in to process queries.
	stringstream strstream(query);
	string token;
	map<string, token_info_t> query_words_info;
	map<string, Query_hit> hits_map;
	// stat feq
	while (strstream >> token) {
		for_each(token.begin(), token.end(), [](char& ch){ch = tolower(ch);});
		if (stop_words.find(token) != stop_words.end()) {
			continue;
		}
		for(auto it = token.begin(); it != token.end();){
			if (!isalnum(*it)) {
				token.erase(it);
			} else {
				it++;
			}
		}
		if (token == "") {
			continue;
		}
		if (index_map.find(token) == index_map.end()) {
			return;
		}
		query_words_info[token].feq++;
	}
	// calculate normalization factor of the query
	for (auto &word_pair : query_words_info) {
		string temp_word = word_pair.first;
		int feq = word_pair.second.feq;
		normalization_factor += pow(feq*index_map[temp_word].idf, 2);
	}
	// calculate W_ik
	for (auto &word_pair : query_words_info) {
		string temp_word = word_pair.first;
		int feq = word_pair.second.feq;
		word_pair.second.w = feq*index_map[temp_word].idf/sqrt(normalization_factor);
	}
	// calculate Score
	for (auto &word_pair : query_words_info) {
		string temp_word = word_pair.first;
		double w = query_words_info[temp_word].w;
		for (auto it = index_map[temp_word].weights.begin(); it != index_map[temp_word].weights.end(); it++) {
			string id;
			stringstream id_stream;
			id_stream << it->doc_id;
			id_stream >> id;
			auto hit_it = hits_map.find(id);
			if (hit_it != hits_map.end()) {
				hit_it->second.score += w*it->weight;
				hit_it->second.times++;
			} else {
				hits_map.insert(make_pair(id,Query_hit{id,w*it->weight}));
			}
		}
	}
	int query_words_num = query_words_info.size();
	for (auto hit_it = hits_map.begin(); hit_it != hits_map.end(); hit_it++) {
		stringstream sstream(hit_it->second.id);
		int doc_id;
		sstream >> doc_id;
		hit_it->second.score = (weight * pr_map[doc_id]) + (1 - weight) * (hit_it->second.score);
		if (query_words_num > hit_it->second.times) {
			continue;
		}
		hits.push_back(hit_it->second);
	}
	sort(hits.begin(), hits.end(), [](const Query_hit& lhs, const Query_hit& rhs){
		return lhs.score > rhs.score;
	});
	
}

namespace {
	int handle_request(mg_connection *conn)
	{
		const mg_request_info *request_info = mg_get_request_info(conn);
		
		if (!strcmp(request_info->request_method, "GET") && request_info->query_string) {
			// Make the processing of each server request mutually exclusive with
			// processing of other requests.
			
			// Retrieve the request form data here and use it to call search(). Then
			// pass the result of search() to to_json()... then pass the resulting string
			// to mg_printf.
			string query;
			if (get_param(request_info, "q", query) == -1) {
				// If the request doesn't have the "q" field, this is not an index
				// query, so ignore it.
				return 1;
			}
			string weight;
			double w;
			if (get_param(request_info, "w", weight)) {
				return 1;
			}
			stringstream sstream(weight);
			sstream >> w;
			
			vector<Query_hit> hits;
			Index_server *server = static_cast<Index_server *>(request_info->user_data);
			
			pthread_mutex_lock(&mutex);
			server->process_query(query, w, hits);
			pthread_mutex_unlock(&mutex);
			
			string response_data = to_json(hits);
			int response_size = response_data.length();
			
			// Send HTTP reply to the client.
			mg_printf(conn,
					  "HTTP/1.1 200 OK\r\n"
					  "Content-Type: application/json\r\n"
					  "Content-Length: %d\r\n"
					  "\r\n"
					  "%s", response_size, response_data.c_str());
		}
		
		// Returning non-zero tells mongoose that our function has replied to
		// the client, and mongoose should not send client any more data.
		return 1;
	}
	
	int get_param(const mg_request_info *request_info, const char *name, string& param)
	{
		const char *get_params = request_info->query_string;
		size_t params_size = strlen(get_params);
		
		// On the off chance that operator new isn't thread-safe.
		pthread_mutex_lock(&mutex);
		char *param_buf = new char[params_size + 1];
		pthread_mutex_unlock(&mutex);
		
		param_buf[params_size] = '\0';
		int param_length = mg_get_var(get_params, params_size, name, param_buf, params_size);
		if (param_length < 0) {
			return param_length;
		}
		
		// Probably not necessary, just a precaution.
		param = param_buf;
		delete[] param_buf;
		
		return 0;
	}
	
	// Converts the supplied query hit list into a JSON string.
	string to_json(const vector<Query_hit>& hits)
	{
		ostringstream os;
		os << "{\"hits\":[";
		vector<Query_hit>::const_iterator viter;
		for (viter = hits.begin(); viter != hits.end(); ++viter) {
			if (viter != hits.begin()) {
				os << ",";
			}
			
			os << *viter;
		}
		os << "]}";
		
		return os.str();
	}
	
	// Outputs the computed information for a query hit in a JSON format.
	ostream& operator<< (ostream& os, const Query_hit& hit)
	{
		os << "{" << "\"id\":\"";
		int id_size = hit.id.size();
		for (int i = 0; i < id_size; i++) {
			if (hit.id[i] == '"') {
				os << "\\";
			}
			os << hit.id[i];
		}
		return os << "\"," << "\"score\":" << hit.score << "}";
	}
}
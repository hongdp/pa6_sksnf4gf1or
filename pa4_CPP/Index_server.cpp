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

#include "mongoose.h"

using namespace std;

namespace {
    int handle_request(mg_connection *);
    int get_param(const mg_request_info *, const char *, string&);
    string get_param(const mg_request_info *, const char *);
    string to_json(const vector<Query_hit>&);

    ostream& operator<< (ostream&, const Query_hit&);
}

struct weight{
	int doc_id;
	double weight;
};

struct word_info{
	double idf;
	vector <weight> weights;
};

map <string,word_info> index_map;
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
    while(getline(infile, index)){
		
      
    }
    // Fill in this method to load the inverted index from disk.
	
}

// Search the index for documents matching the query. The results are to be
// placed in the supplied "hits" vector, which is guaranteed to be empty when
// this method is called.
void Index_server::process_query(const string& query, vector<Query_hit>& hits)
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
		if (index_map.find(token) == index_map.end()) {
			continue;
		}
		query_words_info[token].feq++;
	}
	// calculate normalization factor of the query
	for (auto word_pair : query_words_info) {
		string temp_word = word_pair.first;
		int feq = word_pair.second.feq;
		normalization_factor += pow(feq*index_map[temp_word].idf, 2);
	}
	// calculate W_ik
	for (auto word_pair : query_words_info) {
		string temp_word = word_pair.first;
		int feq = word_pair.second.feq;
		word_pair.second.w = feq*index_map[temp_word].idf/sqrt(normalization_factor);
	}
	// calculate Score
	for (auto word_pair : query_words_info) {
		string temp_word = word_pair.first;
		int w = query_words_info[temp_word].w;
		for (auto it = index_map[temp_word].weights.begin(); it != index_map[temp_word].weights.end(); it++) {
			string id;
			stringstream id_stream(it->doc_id);
			id_stream >> id;
			auto hit_it = hits_map.find(id);
			if (hit_it != hits_map.end()) {
				hit_it->second.score += w*it->weight;
			} else {
				hits_map.insert(make_pair(id,Query_hit{id,w*it->weight}));
			}
		}
	}
	
	for (auto hit_it = hits_map.begin(); hit_it != hits_map.end(); hit_it++) {
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

            vector<Query_hit> hits;
            Index_server *server = static_cast<Index_server *>(request_info->user_data);

            pthread_mutex_lock(&mutex);
            server->process_query(query, hits);
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

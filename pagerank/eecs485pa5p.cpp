#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <map>
#include <string>
#include <iomanip>
#include "PageNode.h"

using namespace std;

int extract_vertex(string line) {
	assert(!line.empty());
	size_t idx_first_larger_sign = line.find_first_of('>');
	size_t idx_last_smaller_sign = line.find_last_of('<');
	assert(idx_last_smaller_sign > idx_first_larger_sign);
	size_t str_length = idx_last_smaller_sign - idx_first_larger_sign - 1;
	string number = line.substr(idx_first_larger_sign+1, str_length);
	int result = atoi(number.c_str());
	return result;
}

PageNode* get_Node_ptr(int node_id, map<int, PageNode*>& Node_map, int& num_ver) {
	PageNode* result;
	if (Node_map.find(node_id) == Node_map.end()) {
		result = new PageNode();
		num_ver++;
		Node_map[node_id] = result;
	} else {
		result = Node_map[node_id];
	}
	assert(result);
	return result;
}

int main(int argc, char const *argv[]) {
	if (argc != 6) {
		cout << "Usage: eecs485pa5p <dvalue> (-k <numiterations> |";
		cout <<" -converge <maxchange>) inputnetfile outputfile" << endl;
		exit(1);
	}
	assert(argv);

	// init with input arguement
	double d = atof(argv[1]);
	string converge_type(argv[2]);
	double converge_val = atof(argv[3]);
	string IN_FILE(argv[4]);
	string OUT_FILE(argv[5]);

	// define xml tag
	const string EDGES_TAG = "<eecs485_edges>";
	const string EDGES_CLOSING_TAG = "</eecs485_edges>";
	const string EDGE_TAG = "<eecs485_edge>";
	const string EDGE_CLOSING_TAG = "</eecs485_edge>";
	const string FROM_TAG = "<eecs485_from>";
	const string FROM_CLOSING_TAG = "</eecs485_from>";
	const string TO_TAG = "<eecs485_to>";
	const string TO_CLOSING_TAG = "</eecs485_to>";

	// Read file
	map<int, PageNode*> Node_map;
	ifstream infile_stream(IN_FILE.c_str());
	int num_ver = 0;
	if (!infile_stream.is_open()) {
		cout << "Failed to open input file" << endl;
		exit(1);
	}
	string root_tag;
	infile_stream >> root_tag;
	if (root_tag != EDGES_TAG) {
		cout << "Wrong root tag, not " << EDGES_TAG << endl;
		exit(1);
	}
	// read Vertices and Edges, read four lines at a time
	#ifdef DEBUG
	int num_edge = 0;
	#endif
	string line;
	while (infile_stream >> line) {
		if (line == EDGES_CLOSING_TAG) {
			#ifdef DEBUG
			cout << "Reaching " << EDGES_CLOSING_TAG << endl;
			#endif
			break;
		}
		#ifdef DEBUG
		num_edge++;
		if (!(num_edge%100000)) {
			cout << "read edge No. " << num_edge << endl;
		}
		#endif
		if (line != EDGE_TAG) {
			cout << "Missing " << EDGE_TAG << endl;
			exit(1);
		}
		string from_line, to_line, edge_closing_tag;
		infile_stream >> from_line;
		infile_stream >> to_line;
		infile_stream >> edge_closing_tag;
		if (edge_closing_tag != EDGE_CLOSING_TAG) {
			cout << "Missing " << EDGE_CLOSING_TAG << endl;
			exit(1);
		}
		int src, dest;
		// read vertices from lines
		src = extract_vertex(from_line);
		dest = extract_vertex(to_line);
		if (src == dest) {
			continue;
		}
		// allocate vertice if not exist yet
		PageNode* src_ptr = get_Node_ptr(src, Node_map, num_ver);
		PageNode* dest_ptr = get_Node_ptr(dest, Node_map, num_ver);
		dest_ptr->addContributor(src_ptr);
		src_ptr->addOutLink();
	}
	#ifdef DEBUG
	cout << "Total num of node: " << num_ver << endl;
	cout << "Total num of edge: " << num_edge << endl;
	#endif
	infile_stream.close();

	// set class global variables before calculation
	PageNode::setGlobalArgs(d, num_ver);

	// iterate through the map and initialize the initPR, and add noOutPageNode
	for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
		it->second->initPR(1/double(num_ver));
		if (it->second->hasNoOutGoing()) {
			PageNode::addNoOut();
		}
	}

	// determine converge type and start iteration
	bool use_iter = (converge_type == "-k");
	if (use_iter) {
		for (size_t i = size_t(converge_val); i > 0; i--) {
			#ifdef DEBUG
			cout << "Iteration No." << size_t(converge_val) - i + 1 << endl;
			#endif
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
				// cout << "Calculating Node No." << it->first << endl;
				it->second->calculateNextPR();
			}
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
				// cout << "Updating Node No." << it->first << endl;
				it->second->updatePR();
			}
			PageNode::updateNoOutPR();
		}
	} else {
		bool break_loop = false;
		#ifdef DEBUG
		int iteration = 0;
		#endif
		while(true) {
			#ifdef DEBUG
			iteration++;
			cout << "Iteration NO." << iteration << endl;
			#endif
			break_loop = true;
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
				it->second->calculateNextPR();
			}
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
				break_loop &= it->second->updatePR(converge_val);
			}
			PageNode::updateNoOutPR();
			if (break_loop)
				break;
		}
	}

	// output file
	ofstream outfile_stream(OUT_FILE.c_str());
    outfile_stream << std::setprecision(6);
#ifdef DEBUG
	double totalRP = 0;
#endif
	for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
#ifdef DEBUG
		totalRP += it->second->getCurrentPR();
#endif
		outfile_stream << it->first << "," << it->second->getCurrentPR() << endl;
	}
#ifdef DEBUG
	cout << totalRP << endl;
#endif
	outfile_stream.close();

	for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
		delete it->second;
	}

	return 0;
}

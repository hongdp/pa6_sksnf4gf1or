#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <map>
#include <iomanip>
#include "PageNode.h"

using namespace std;

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

	// Read file
	map<int, PageNode*> Node_map;
	ifstream infile_stream(IN_FILE.c_str());
	string ver_title;
	int num_ver;
	if (!infile_stream.is_open()) {
		cout << "Failed to open input file" << endl;
		exit(1);
	}
	// read Vertices
	infile_stream >> ver_title >> num_ver;
	if (ver_title != "*Vertices") {
		cout << "Wrong file format! No *Vertices" << endl;
		exit(1);
	}
	PageNode::setGlobalArgs(d, num_ver);
#ifdef DEBUG
	cout << "Total num of node: " << num_ver << endl;
#endif
	for (size_t i = 0; i < size_t(num_ver); i++) {
#ifdef DEBUG
		if (!(i%100000)) {
			cout << "read in No." << i << " node" << endl;
		}
#endif
		PageNode* tmp_node = new PageNode(1/double(num_ver));
		int id;
		string name;
		infile_stream >> id >> name;
		Node_map[id] = tmp_node;
	}

	// read edges
	string edge_title;
	int num_edge;
	infile_stream >> edge_title >> num_edge;
	if (edge_title != "*Arcs") {
		cout << "Wrong file format! No *Arcs" << endl;
		exit(1);
	}
#ifdef DEBUG
	cout << "Total num of edge: " << num_edge << endl;
#endif
	for (size_t i = 0; i < size_t(num_edge); i++) {
#ifdef DEBUG
		if (!(i%100000)) {
			cout << "read in No." << i << " edge" << endl;
		}
#endif
		int src, dest;
		infile_stream >> src >> dest;
		if (src == dest) {
			continue;
		}
		PageNode* src_ptr = Node_map[src];
		PageNode* dest_ptr = Node_map[dest];
		dest_ptr->addContributor(src_ptr);
		src_ptr->addOutLink();
	}
	infile_stream.close();

	for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
		if (it->second->hasNoOutGoing()) {
//			cout << "Adding edge for node No." << it->first << endl;
			PageNode::addNoOut();
//			for (auto destIt = Node_map.begin(); destIt != Node_map.end(); destIt++) {
//				if (destIt->second != it->second) {
//					it->second->addOutLink();
//					destIt->second->addContributor(it->second);
//				}
//			}
		}
	}
	// determine converge type
	bool use_iter = (converge_type == "-k");
	if (use_iter) {
		for (size_t i = size_t(converge_val); i > 0; i--) {
			#ifdef DEBUG
			cout << "Iteration No." << size_t(converge_val) - i + 1 << endl;
			#endif
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
//				cout << "Calculating Node No." << it->first << endl;
				it->second->calculateNextPR();
			}
			for (auto it = Node_map.begin(); it != Node_map.end(); it++) {
//				cout << "Updating Node No." << it->first << endl;
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

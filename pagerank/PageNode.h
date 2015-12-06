//
//  PageNode.hpp
//  PageRankProj
//
//  Created by Dapeng Hong on 11/20/15.
//  Copyright © 2015 Hongdp. All rights reserved.
//

#ifndef PageNode_H
#define PageNode_H

#include <vector>

class PageNode {
public:
	PageNode():currentPR(0.0), nextPR(0), numOutLinks(0) {};
	void initPR(double initPR);
	void addOutLink();
	void addContributor(const PageNode* contributor_ptr);
	void calculateNextPR();
	double getCurrentPR() const;
	double getContribution() const;
	void updatePR();
	bool updatePR(double converge);
	bool hasNoOutGoing();
	static void setGlobalArgs(double d, int N);
	static void addNoOut();
	static void updateNoOutPR();

private:
	double currentPR;
	double nextPR;
	std::vector<const PageNode*> contributors;
	static int numNodesWithNoOut;
	int numOutLinks;
	static double d;
	static int N;
	static double currentTotalNoOutPR;
	static double nextTotalNoOutPR;
};

#endif /* PageNode_H */

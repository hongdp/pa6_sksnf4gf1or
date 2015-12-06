//
//  PageNode.cpp
//  PageRankProj
//
//  Created by Dapeng Hong on 11/20/15.
//  Copyright © 2015 Hongdp. All rights reserved.
//

#include "PageNode.h"

#include <vector>
#include <cmath>
#include <cstdio>

using namespace std;

double PageNode::d;
int PageNode::N;
double PageNode::currentTotalNoOutPR = .0;
double PageNode::nextTotalNoOutPR = .0;
int PageNode::numNodesWithNoOut = 0;

void PageNode::initPR(double initPR) {
	// printf("%f\n", initPR );
	currentPR = initPR;
}

void PageNode::addOutLink(){
	numOutLinks++;
}

void PageNode::calculateNextPR(){
	nextPR = (1 - d)/N;
	for (const PageNode* nodePtr : contributors) {
		nextPR += d*nodePtr->getContribution();
	}
	if (numOutLinks == 0) {
		nextPR += d*(currentTotalNoOutPR - currentPR)/(N - 1);
		nextTotalNoOutPR += nextPR;
	} else {
		nextPR += d*currentTotalNoOutPR/(N - 1);
	}
}

double PageNode::getCurrentPR() const {
	return currentPR;
}

void PageNode::updatePR(){
	currentPR = nextPR;
	nextPR = 0;
}

void PageNode::updateNoOutPR(){
	currentTotalNoOutPR = nextTotalNoOutPR;
	nextTotalNoOutPR = 0;
}

bool PageNode::updatePR(double converge){
	bool returnVal = false;
	if (abs(currentPR - nextPR)/currentPR <= converge ) {
		returnVal = true;
	}
	currentPR = nextPR;
	nextPR = 0;
	return returnVal;
}

double PageNode::getContribution() const{
	return currentPR/numOutLinks;
}

void PageNode::setGlobalArgs(double d_, int N_){
	d = d_;
	N = N_;
}

void PageNode::addContributor(const PageNode* contributor_ptr){
	contributors.push_back(contributor_ptr);
}

bool PageNode::hasNoOutGoing(){
	return !numOutLinks;
}

void PageNode::addNoOut(){
	numNodesWithNoOut++;
	currentTotalNoOutPR += 1.0/N;
}

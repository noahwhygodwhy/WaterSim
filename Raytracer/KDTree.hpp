#ifndef KDTREE_H
#define KDTREE_H


#include <vector>
#include <algorithm>
#include <queue>
#include <iostream>
#include "opencl.hpp"
#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/integer.hpp>
#include <glm/gtx/norm.hpp>
#define CPP
#include "Shared.cl"

using namespace std;
using namespace glm;


enum Axis {
	X = 0,
	Y,
	Z
};

//struct KDNode {	
//	float value = -1.0f;
//	uint32_t pointIdx = 0; //index into balls
//	int32_t greaterChild = -1; //index into the tree
//	int32_t lesserChild = -1;//index into the tree
//};



struct KDConstructionContext {
	const Particle* particles;
	const size_t numberOfPoints;
	cl_mem* clTotalList;
	cl_mem* clQA;
	cl_mem* clQB;
	cl_mem* clTheTree;
	cl_command_queue* cmdQueue;
	cl_kernel* kdTreeKernel;
	cl_kernel* bitonicKernel;
	cl_mem* clnodeIndexList;
};



void printTree(KDNode* theTree);


void makeKDTree(const Particle* balls, const size_t& numberOfPoints, const KDConstructionContext& kdConCon);

vector<int32_t> getDotsInRange(const Particle* balls, KDNode* theTree, uint32_t originDot, float range);

vector<int32_t> getDotsInRangeOld(const Particle* balls, KDNode* theTree, uint32_t originDot, float range);


#endif
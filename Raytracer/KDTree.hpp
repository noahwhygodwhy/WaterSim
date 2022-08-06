#ifndef KDTREE_H
#define KDTREE_H


#include <vector>
#include <algorithm>
#include <queue>
#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/integer.hpp>
#include <glm/gtx/norm.hpp>
using namespace std;
using namespace glm;


enum Axis {
	X = 0,
	Y,
	Z
};

//struct AABB {
//	fvec3 min;
//	fvec3 max;
//	fvec3  middle() const {
//		return (min + max) / 2.0f;
//	}
//};

struct KDNode {
	float value = -1.0f;
	uint32_t pointIdx = 0; //index into balls
	int32_t greaterChild = -1; //index into the tree
	int32_t lesserChild = -1;//index into the tree
};


//AABB pointAABB(const fvec3& pos, const float rad) {
//	return AABB(pos - fvec3(rad), pos + fvec3(rad));
//
//}


void printTree(KDNode* theTree);


KDNode* makeKDTree(const fvec4* balls, const size_t& numberOfPoints);
//void getDotsInRange(vector<int32_t>& outputVector, const fvec4* balls, KDNode* theTree, uint32_t originDot, float range, const int32_t receivedIndex = 1, uint32_t layer = 0);
vector<int32_t> getDotsInRange(const fvec4* balls, KDNode* theTree, uint32_t originDot, float range);
/*

0b00000...00ZYX

nnn
nnp
npn
npp
pnn
pnp
ppn
ppp
*/
//
//void processPoints(const AABB& currAABB,
//	const fvec3* balls,
//	size_t* pointList, //the index into balls and aabbList
//	AABB* aabbList,
//	const size_t& numberOfPoints,
//	const float& pointRadius,
//	size_t layer
//) {
//
//	vector<size_t>* nnn = new vector<size_t>[8];
//
//	for (int i = 0; i < 8; i++) {//is this needed? or am i dumb
//		nnn[i] = vector<size_t>();
//	}
//
//
//	fvec3 mid = currAABB.middle();
//
//	uint8_t greater, lesser, intersected;
//
//	for (size_t i = 0; i < numberOfPoints; i++) {
//
//		greater = (uint8_t(aabbList->max.z >= mid.z) << int(Z)) | (uint8_t(aabbList->max.y >= mid.y) << int(Y)) | (uint8_t(aabbList->max.x >= mid.x) << int(X));
//		lesser = (uint8_t(aabbList->min.z <= mid.z) << int(Z)) | (uint8_t(aabbList->min.y <= mid.y) << int(Y)) | (uint8_t(aabbList->min.x <= mid.x) << int(X));
//		uint8_t intersected = greater & lesser;
//
//		if (intersected) {
//			//add it to "curr" node
//		}
//		else {
//			nnn[greater].push_back(pointList[i]);
//		}
//
//
//
//	}
//}
//




#endif
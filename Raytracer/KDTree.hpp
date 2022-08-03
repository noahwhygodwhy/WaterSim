#ifndef KDTREE_H
#define KDTREE_H


#include "glm/glm.hpp"
#include <vector>
#include <algorithm>
#include "glm/gtx/string_cast.hpp"
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
	float value;
	uint32_t pointIdx;
	int32_t greaterChild;
	int32_t lesserChild;
};

struct KDTree {
	KDNode* tree;
	uint32_t layers;
	uint32_t size;
};


//AABB pointAABB(const fvec3& pos, const float rad) {
//	return AABB(pos - fvec3(rad), pos + fvec3(rad));
//
//}





KDTree makeKDTree(const fvec3* balls, const size_t& numberOfPoints);
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
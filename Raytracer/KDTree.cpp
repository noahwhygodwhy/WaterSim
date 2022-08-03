#include "KDTree.hpp"

//TODO: multithread later





void processNode(KDNode* theTree, const fvec3* balls, const size_t& numberOfPoints, uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{
	uint32_t axis = layer % 3;
	auto mySert = [balls, axis](size_t a, size_t b) {return balls[a][0] < balls[b][0]; };
	std::sort(totalList, totalList + numberOfPoints, mySert);


	uint32_t middleIdx = numberOfPoints / 2;
	//TODO: if there are no points above middleIdx; 
	//todo: if there are no points below middleidx;

	int32_t lesserChildIdx = -1;
	int32_t greaterChildIdx = -1;
	uint32_t* lesserList = 0;
	uint32_t* greaterList = 0;

	if (middleIdx > 0) {//has lesser children
		lesserList = new uint32_t[middleIdx]();
		lesserChildIdx = receivedIndex << 1;
	}
	if (middleIdx < numberOfPoints - 1) { //has greater children
		greaterList = new uint32_t[numberOfPoints - middleIdx]();
		greaterChildIdx = (receivedIndex << 1) + 1;
	}
	theTree[receivedIndex] = KDNode(
		balls[totalList[middleIdx]][axis],
		totalList[middleIdx],
		greaterChildIdx,
		lesserChildIdx
	);

	if (lesserList) {//has lesser children
		memcpy(lesserList, totalList, middleIdx);
	}
	if (greaterList) { //has greater children
		memcpy(greaterList, totalList + middleIdx + 1, numberOfPoints - middleIdx);
	}
	delete[] totalList;
	
	if (lesserList) {//has lesser children
		processNode(theTree, balls, middleIdx, lesserList, layer + 1, lesserChildIdx);
	}
	if (greaterList) { //has greater children
		processNode(theTree, balls, numberOfPoints - middleIdx, greaterList, layer + 1, greaterChildIdx);
	}
}









KDNode* makeKDTree(const fvec3* balls, const size_t& numberOfPoints) {
	uint32_t* totalList = new uint32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i] = i;	
	}

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(numberOfPoints))));
	printf("number of nodes for memory: %lu\n", memForTree);
	KDNode* toReturn = new KDNode[memForTree]();
	processNode(toReturn, balls, numberOfPoints, totalList, 0, 0);
	return toReturn;
}


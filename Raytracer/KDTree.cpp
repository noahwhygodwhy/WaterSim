#include "KDTree.hpp"
#include <queue>

void printTree(KDNode* theTree) {
	const char* axises = "XYZ";
	int layer = 0;
	bool hasNextLayer = true;
	while (hasNextLayer && layer < 6) {
		hasNextLayer = false;
		size_t firstIndex = size_t(glm::pow(2, layer));

		char axis = axises[layer % 3];

		for (int i = 0; i < glm::pow(2, layer); i++) {
			KDNode curr = theTree[firstIndex + i];
			if (curr.greaterChild >= 0 || curr.lesserChild >= 0) {
				hasNextLayer = true;
			}
			for (int i = 0; i < layer; i++) {
				printf("  ");
			}
			printf("%lu-%c%f\n", curr.pointIdx, axis, curr.value);
		}
		layer++;
	}
}


//TODO: multithread later by turning it into a queue

void processNode(KDNode* theTree, const fvec3* balls, const size_t& numberOfPoints, uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{
	uint32_t axis = layer % 3;
	auto mySert = [balls, axis](size_t a, size_t b) {return balls[a][axis] < balls[b][axis]; };
	std::sort(totalList, totalList + numberOfPoints, mySert);

	uint32_t middleIdx = numberOfPoints / 2;

	int32_t lesserChildIdx = -1;
	int32_t greaterChildIdx = -1;
	uint32_t* lesserList = 0;
	uint32_t* greaterList = 0;

	if (middleIdx > 0) {//has lesser children
		lesserList = new uint32_t[middleIdx]();
		lesserChildIdx = receivedIndex << 1;
	}
	if (middleIdx < numberOfPoints - 1) { //has greater children
		greaterList = new uint32_t[numberOfPoints - middleIdx - 1]();
		greaterChildIdx = (receivedIndex << 1) + 1;
	}
	theTree[receivedIndex-1] = KDNode(
		balls[totalList[middleIdx]][axis],
		totalList[middleIdx],
		greaterChildIdx,
		lesserChildIdx
	);

	if (lesserList) {//has lesser children
		memcpy(lesserList, totalList, middleIdx*sizeof(int32));
	}
	if (greaterList) { //has greater children
		memcpy(greaterList, totalList + middleIdx + 1, ((numberOfPoints - middleIdx) - 1) * sizeof(int32));
	}

	//printf("\n");
	delete[] totalList;
	
	if (lesserList) {//has lesser children
		processNode(theTree, balls, middleIdx, lesserList, layer + 1, lesserChildIdx);
	}
	if (greaterList) { //has greater children
		processNode(theTree, balls, numberOfPoints - middleIdx - 1, greaterList, layer + 1, greaterChildIdx);
	}
}




KDNode* makeKDTree(const fvec3* balls, const size_t& numberOfPoints) {
	uint32_t* totalList = new uint32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i ] = i;	
	}

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(numberOfPoints))));
	printf("number of nodes for memory: %zu\n", memForTree);
	KDNode* toReturn = new KDNode[memForTree]();
	processNode(toReturn, balls, numberOfPoints, totalList, 0, 1);
	return toReturn;
}




//get nearest neighbors https://en.wikipedia.org/wiki/K-d_tree#Nearest_neighbour_search
void getDotsInRange(vector<uint32_t>& outputVector, const fvec3* balls, KDNode* theTree, const size_t receivedIndex, uint32_t originDot, float range, uint32_t layer = 0) {
	uint32_t axis = layer % 3;
	fvec3 theBall = balls[originDot];
	KDNode currNode = theTree[receivedIndex];

	//this might be less than efficient, but allows for weirdness with duplicate values

	int32_t newIndex = (int32_t(theBall[axis] > currNode.value) * currNode.greaterChild) + (int32_t(theBall[axis] < currNode.value) * currNode.lesserChild);
	if (newIndex == 0) {
		printf("something's wrong in getsdotsinrange newIndex\n");
		exit(0);
	}
	if (newIndex > 0) {
		getDotsInRange(outputVector, balls, theTree, uint32_t(newIndex), originDot, range, layer + 1);
	}

	//TODO: finish it




}


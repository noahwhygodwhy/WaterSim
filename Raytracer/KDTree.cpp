#include "KDTree.hpp"
#include <queue>
//TODO: multithread later


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


void processNode(KDNode* theTree, const fvec3* balls, const size_t& numberOfPoints, uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{
	printf("\n\ndoing node on layer %u at index %zu\n", layer, receivedIndex);
	for (int i = 0; i < numberOfPoints; i++) {
		printf("%u, ", totalList[i]);
	}
	printf("\n");
	uint32_t axis = layer % 3;
	auto mySert = [balls, axis](size_t a, size_t b) {return balls[a][axis] < balls[b][axis]; };
	std::sort(totalList, totalList + numberOfPoints, mySert);

	for (int i = 0; i < numberOfPoints; i++) {
		printf("%u, ", totalList[i]);
	}
	printf("\n");

	uint32_t middleIdx = numberOfPoints / 2;
	printf("middleidx=%i\n", middleIdx);

	int32_t lesserChildIdx = -1;
	int32_t greaterChildIdx = -1;
	uint32_t* lesserList = 0;
	uint32_t* greaterList = 0;

	if (middleIdx > 0) {//has lesser children
		printf("it has lesser children\n");
		lesserList = new uint32_t[middleIdx]();
		lesserChildIdx = receivedIndex << 1;
	}
	if (middleIdx < numberOfPoints - 1) { //has greater children
		printf("it has greater children\n");
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
		printf("lesser list:");
		for (int i = 0; i < middleIdx; i++) {
			printf("%i, ", lesserList[i]);
		}
		printf("\n");
	}
	if (greaterList) { //has greater children
		memcpy(greaterList, totalList + middleIdx + 1, ((numberOfPoints - middleIdx) - 1) * sizeof(int32));
		printf("greater list:");
		for (int i = 0; i < numberOfPoints - middleIdx - 1; i++) {
			printf("%i, ", greaterList[i]);
		}
		printf("\n");
	}

	printf("\n");
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
		totalList[i] = i;	
	}

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(numberOfPoints))));
	printf("number of nodes for memory: %lu\n", memForTree);
	KDNode* toReturn = new KDNode[memForTree]();
	processNode(toReturn, balls, numberOfPoints, totalList, 0, 1);
	return toReturn;
}


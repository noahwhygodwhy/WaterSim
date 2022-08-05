#include "KDTree.hpp"
#include <queue>

void printTree(KDNode* theTree) {
	const char* axises = "XYZ";
	int layer = 0;
	bool hasNextLayer = true;


	queue<KDNode> printQueue = queue<KDNode>();
	printQueue.push(theTree[0]);

	int i = 0; 
	while (!printQueue.empty()) {
		int spaces = ceil(glm::log2(i+1));
		KDNode curr = printQueue.front();
		printQueue.pop();
		for (int k = 0; k < spaces; k++) {
			printf("  ");
		}
		printf("i: %i, L:%i, R:%i\n",i, curr.lesserChild, curr.greaterChild);
		if (curr.lesserChild > 0) {
			printQueue.push(theTree[curr.lesserChild]);
		}
		if (curr.greaterChild > 0) {
			printQueue.push(theTree[curr.greaterChild]);
		}
		i++;
	}


	//while (hasNextLayer && layer < 6) {




	//	//printf("on layer %i\n", layer);
	//	hasNextLayer = false;
	//	size_t firstIndex = size_t(glm::pow(2, layer));

	//	char axis = axises[layer % 3];

	//	for (int i = 0; i < glm::pow(2, layer); i++) {
	//		int32_t indexIntoTree = (firstIndex + i)-1;
	//		//printf("index into tree; %i\n", indexIntoTree);
	//		KDNode curr = theTree[firstIndex + i];
	//		if (curr.greaterChild >= 0 || curr.lesserChild >= 0) {
	//			//printf("it has a next layer\n");
	//			hasNextLayer = true;
	//		}
	//		for (int i = 0; i < layer; i++) {
	//			printf("  ");
	//		}
	//		printf("treeIdx: %i, L:%i, R:%i\n", indexIntoTree, curr.lesserChild, curr.greaterChild);
	//	}
	//	layer++;
	//}
}








//TODO: processNode is wrong, misses a point at some point..?







//TODO: multithread later by turning it into a queue

void processNode(KDNode* theTree, const fvec4* balls, const size_t& numberOfPoints, uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{

	uint32_t axis = layer % 3;

	auto mySert = [balls, axis](size_t a, size_t b) {

		return balls[a][axis] < balls[b][axis];
	};


	std::sort(totalList, totalList + numberOfPoints, mySert);

	int32_t middleIdx = numberOfPoints / 2;

	int32_t lesserChildIdx = -1;
	int32_t greaterChildIdx = -1;
	uint32_t* lesserList = 0;
	uint32_t* greaterList = 0;

	if (middleIdx > 0) {//has lesser children
		printf("about to make array with middleIdx: %u\n", middleIdx);
		lesserList = new uint32_t[middleIdx]();
		lesserChildIdx = receivedIndex << 1;
	}
	if (middleIdx < numberOfPoints - 1) { //has greater children
		printf("about to make array with the other: %u\n", numberOfPoints - middleIdx - 1);
		greaterList = new uint32_t[numberOfPoints - middleIdx - 1]();
		greaterChildIdx = (receivedIndex << 1) + 1;
	}
	theTree[receivedIndex-1] = KDNode(
		balls[totalList[middleIdx]][axis],
		totalList[middleIdx],
		greaterChildIdx-1,
		lesserChildIdx-1
	);

	if (lesserList) {//has lesser children
		memcpy(lesserList, totalList, middleIdx*sizeof(int32));
	}
	if (greaterList) { //has greater children
		memcpy(greaterList, totalList + middleIdx + 1, ((numberOfPoints - middleIdx) - 1) * sizeof(int32));
	}

	//printf("\n");
	//delete[] totalList;
	
	if (lesserList) {//has lesser children
		processNode(theTree, balls, middleIdx, lesserList, layer + 1, lesserChildIdx);
	}
	if (greaterList) { //has greater children
		processNode(theTree, balls, numberOfPoints - middleIdx - 1, greaterList, layer + 1, greaterChildIdx);
	}
}




KDNode* makeKDTree(const fvec4* balls, const size_t& numberOfPoints) {
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



void spaces(int32_t i) {
	for (int k = 0; k < i; k++) {
		printf("  ");
	}
}


//get nearest neighbors https://en.wikipedia.org/wiki/K-d_tree#Nearest_neighbour_search
void getDotsInRange(vector<int32_t>& outputVector, const fvec4* balls, KDNode* theTree, uint32_t originDot, float range, const int32_t receivedIndex, uint32_t layer) {
	
	
	for (int k = 0; k < layer; k++) {
		printf("  ");
	}
	printf("on layer %i, doing index %i\n", layer, receivedIndex);
	
	uint32_t axis = layer % 3;
	fvec4 theBall = balls[originDot];
	KDNode currNode = theTree[receivedIndex];


	if ((distance2(theBall, balls[currNode.pointIdx]) <= range*range) && (currNode.pointIdx != originDot)) {

		//printf("%i is close enough to %i, adding it\n", currNode.pointIdx, originDot);
		outputVector.push_back(currNode.pointIdx);
	}

	//if originDot within range of the split, then you have to do both
	//if originDot out of range of the split, and on the wrong side, then you can prune



	bool tooCloseToCall = glm::abs(theBall[axis] - currNode.value) <= range;
	bool possiblyGreater = theBall[axis] > currNode.value;
	bool possiblyLesser = theBall[axis] < currNode.value;

	spaces(layer);
	printf("tooCloseToCall: %i, possiblyGreater: %i, possiblyLesser: %i\n", tooCloseToCall, possiblyGreater, possiblyLesser);


	if (currNode.greaterChild > 0 and (tooCloseToCall || possiblyGreater)) {
		spaces(layer);
		printf("has a greater child at %i", currNode.greaterChild);

		getDotsInRange(outputVector, balls, theTree, originDot, range, currNode.greaterChild, layer + 1);
	}
	if (currNode.greaterChild > 0 and (tooCloseToCall || possiblyLesser)) {
		spaces(layer);
		printf("has a lesser child at %i", currNode.lesserChild);
		getDotsInRange(outputVector, balls, theTree, originDot, range, currNode.lesserChild, layer + 1);
	}
}


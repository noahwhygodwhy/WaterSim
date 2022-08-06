#include "KDTree.hpp"










void spaces(int32_t i) {
	for (int k = 0; k < i; k++) {
		printf("  ");
	}
}


char axises[3] = { 'X', 'Y','Z' };

void printTree(KDNode* theTree) {
	const char* axises = "XYZ";
	int layer = 0;
	bool hasNextLayer = true;


	queue<KDNode> printQueue = queue<KDNode>();
	printQueue.push(theTree[0]);

	int i = 0; 
	while (!printQueue.empty()) {
		KDNode curr = printQueue.front();
		printQueue.pop();
		int layer = ceil(glm::log2(i + 1));

		char axis = axises[layer % 3];
		spaces(layer);
		printf("i: %i, L:%i, R:%i, val: %i, split: %c | %.2f\n",i, curr.lesserChild, curr.greaterChild, curr.pointIdx, axis, curr.value);
		if (curr.lesserChild > 0) {
			printQueue.push(theTree[curr.lesserChild]);
		}
		if (curr.greaterChild > 0) {
			printQueue.push(theTree[curr.greaterChild]);
		}
		i++;
	}
}








//TODO: processNode is wrong, misses a point at some point..?







//TODO: multithread later by turning it into a queue

void processNode(KDNode* theTree, const fvec4* balls, size_t startingIndex, size_t endingIndex, uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{
	//spaces(layer);
	//printf("layer %u, indexInTree: %zu, startingIndex: %zu, endingIndex: %zu\n", layer, receivedIndex, startingIndex, endingIndex);



	uint32_t axis = layer % 3;

	auto mySert = [balls, axis](size_t a, size_t b) {
		return balls[a][axis] < balls[b][axis];
	};


	std::sort(totalList+startingIndex, totalList +endingIndex, mySert);

	int32_t middleIdx = (startingIndex+endingIndex) / 2;

	int32_t lesserChildIdx = -1;
	int32_t greaterChildIdx = -1;

	if (middleIdx > startingIndex) {//has lesser children
		lesserChildIdx = receivedIndex << 1;
	}
	if (middleIdx < endingIndex) { //has greater children
		greaterChildIdx = (receivedIndex << 1) + 1;
	}
	//printf("receivedIndex-1: %i\n", receivedIndex - 1);
	theTree[receivedIndex-1] = KDNode(
		balls[totalList[middleIdx]][axis],
		totalList[middleIdx],
		greaterChildIdx-1,
		lesserChildIdx-1
	);
	
	if (middleIdx > startingIndex) {//has lesser children
		processNode(theTree, balls, startingIndex, middleIdx-1, totalList, layer+1, lesserChildIdx);
	}
	if (middleIdx < endingIndex) { //has greater children
		processNode(theTree, balls, middleIdx+1, endingIndex, totalList, layer + 1, greaterChildIdx);
	}
}




KDNode* makeKDTree(const fvec4* balls, const size_t& numberOfPoints) {
	uint32_t* totalList = new uint32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i ] = i;	
	}

	

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(float(numberOfPoints)))));


	//printf("max balls: %u, memfortree:%zu\n", numberOfPoints, memForTree);



	KDNode* toReturn = new KDNode[memForTree]();
	processNode(toReturn, balls, 0, numberOfPoints-1, totalList, 0, 1);
	return toReturn;
}




bool dbg = false;



vector<int32_t> getDotsInRange(const fvec4* balls, KDNode* theTree, uint32_t originDot, float range) {
	if(dbg)printf("\n\nstarting to find dots in range %.2f\n", range);
	const char* axises = "XYZ";
	int layer = 0;
	bool hasNextLayer = true;


	queue<KDNode> printQueue = queue<KDNode>();
	printQueue.push(theTree[0]);

	int i = 0;

	vector<int32_t> outputVector = vector<int32_t>();

	fvec4 theBall = balls[originDot];


	if (dbg)printf("origin: %s\n", glm::to_string(theBall).c_str());

	while (!printQueue.empty()) {
		KDNode currNode = printQueue.front();
		printQueue.pop();


		/*printf("the three numbers:\n%f\n%f\n%f\n",
			float(i + 1),
			glm::log2(float(i + 1)),
			glm::ceil(glm::log2(float(i + 1)))
			);*/


		int layer = int(glm::ceil(glm::log2(float(i + 1)))); //this might fall apart if the tree isn't perfectly balanced, but I think it is so
		int axis = layer % 3;

		spaces(layer);
		printf("i: %i, L:%i, R:%i, val: %i, split: %c | %.2f\n", i, currNode.lesserChild, currNode.greaterChild, currNode.pointIdx, axises[axis], currNode.value);


		if (dbg)spaces(layer-1);
		if (dbg)printf("layer: %i: %c | %.2f\n", layer, axises[axis], currNode.value);

		//spaces(layer);
		//printf("layer: %i, L: %i, R: %i\n", layer, currNode.lesserChild, currNode.greaterChild);
		if (dbg)spaces(layer);
		if (dbg)printf("comparing %i and %i\n", currNode.pointIdx, originDot);
		if (dbg)spaces(layer);
		if (dbg)printf("other: %s\n", glm::to_string(balls[currNode.pointIdx]).c_str());
		if (dbg)spaces(layer);
		if (dbg)printf("distance is %f\n", distance(theBall, balls[currNode.pointIdx]));

		float dsquared = distance2(fvec3(theBall.xyz), fvec3(balls[currNode.pointIdx].xyz));

		if ( (dsquared <= (range * range)) && (currNode.pointIdx != originDot)) {
			if (dbg)printf("puting node %i\n", currNode.pointIdx);
			outputVector.push_back(currNode.pointIdx);
		}


		bool tooCloseToCall = glm::abs(theBall[axis] - currNode.value) <= range;

		if (dbg)spaces(layer);
		if (dbg)printf("too close to call: %s\n", tooCloseToCall ? "True" : "False");

		bool possiblyGreater = theBall[axis] > currNode.value;
		bool possiblyLesser = theBall[axis] < currNode.value;


		if (currNode.lesserChild > 0 && (tooCloseToCall || possiblyLesser)) {
			if (dbg)spaces(layer);
			if (dbg)printf("doing the lesser side %i\n", currNode.lesserChild);
			printQueue.push(theTree[currNode.lesserChild]);
		}
		if (currNode.greaterChild > 0 && (tooCloseToCall || possiblyGreater)) {
			if (dbg)spaces(layer);
			if (dbg)printf("doing the greater side %i\n", currNode.greaterChild);
			printQueue.push(theTree[currNode.greaterChild]);
		}
		i++;
	}
	return outputVector;
}

////get nearest neighbors https://en.wikipedia.org/wiki/K-d_tree#Nearest_neighbour_search
//void getDotsInRange(vector<int32_t>& outputVector, const fvec4* balls, KDNode* theTree, uint32_t originDot, float range, const int32_t receivedIndex, uint32_t layer) {
//	
//	
//	for (int k = 0; k < layer; k++) {
//		printf("  ");
//	}
//	printf("on layer %i, doing index %i\n", layer, receivedIndex);
//	
//	uint32_t axis = layer % 3;
//	fvec4 theBall = balls[originDot];
//	KDNode currNode = theTree[receivedIndex];
//
//
//	if ((distance2(theBall, balls[currNode.pointIdx]) <= range*range) && (currNode.pointIdx != originDot)) {
//		outputVector.push_back(currNode.pointIdx);
//	}
//
//	bool tooCloseToCall = glm::abs(theBall[axis] - currNode.value) <= range;
//	bool possiblyGreater = theBall[axis] > currNode.value;
//	bool possiblyLesser = theBall[axis] < currNode.value;
//
//	spaces(layer);
//	printf("tooCloseToCall: %i, possiblyGreater: %i, possiblyLesser: %i\n", tooCloseToCall, possiblyGreater, possiblyLesser);
//
//
//	if (currNode.greaterChild > 0 and (tooCloseToCall || possiblyGreater)) {
//		spaces(layer);
//		printf("has a greater child at %i", currNode.greaterChild);
//
//		getDotsInRange(outputVector, balls, theTree, originDot, range, currNode.greaterChild, layer + 1);
//	}
//	if (currNode.greaterChild > 0 and (tooCloseToCall || possiblyLesser)) {
//		spaces(layer);
//		printf("has a lesser child at %i", currNode.lesserChild);
//		getDotsInRange(outputVector, balls, theTree, originDot, range, currNode.lesserChild, layer + 1);
//	}
//}
//

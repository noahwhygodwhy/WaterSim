#include "KDTree.hpp"



char axises[3] = { 'X', 'Y','Z' };

void spaces(int32_t i) {
	for (int k = 0; k < i; k++) {
		printf("  ");
	}
}


void printTree(KDNode* theTree) {
	printf("the tree: \n");
	const char* axises = "XYZ";
	int layer = 0;
	bool hasNextLayer = true;


	queue<uint32_t> printQueue = queue<uint32_t>();
	//queue<KDNode> printQueue = queue<KDNode>();
	printQueue.push(0);

	while (!printQueue.empty()) {
		uint32_t currIdx = printQueue.front();
		KDNode curr = theTree[currIdx];
		printQueue.pop();


		int layer = floor(glm::log2(float(currIdx+1)));

		char axis = axises[layer % 3];
		
		spaces(layer);
		printf("layer: %i, idx: %u, L:%i, R:%i, val: %i, split: %c | %.2f\n", layer, currIdx, curr.lesserChild, curr.greaterChild, curr.pointIdx, axis, curr.value);
		if (curr.lesserChild > 0) {
			printQueue.push(curr.lesserChild);
		}
		if (curr.greaterChild > 0) {
			printQueue.push(curr.greaterChild);
		}
	}
}

KDNode* makeKDTree(const fvec4* balls, const size_t& numberOfPoints) {
	uint32_t* totalList = new uint32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i] = i;	
	}

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(float(numberOfPoints)))));
	KDNode* theTree = new KDNode[memForTree]();

	queue<ivec3> indexQueue = queue<ivec3>(); //where each vec3 is (treeIdx, staringIdx, endingIdx)
	
	indexQueue.push(ivec3(1, 0, numberOfPoints - 1));

	while (!indexQueue.empty()) {

		ivec3 curr = indexQueue.front();
		indexQueue.pop();

		int treeIdx = curr.x;
		int startingIdx = curr.y;
		int endingIdx = curr.z;
		int middleIdx = (startingIdx + endingIdx) / 2;

		int layer = int(glm::floor(glm::log2(float(treeIdx))));
		int axis = layer % 3;

		
		int lesserChildIdx = -1;
		int greaterChildIdx = -1;

		auto mySert = [balls, axis](size_t a, size_t b) {
			return balls[a][axis] < balls[b][axis];
		};

		sort(totalList + startingIdx, totalList + endingIdx + 1, mySert);

		if (middleIdx > startingIdx) {//has lesser children
			lesserChildIdx = treeIdx << 1;
			indexQueue.push(ivec3(lesserChildIdx, startingIdx, middleIdx - 1));
		}
		if (middleIdx < endingIdx) { //has greater children
			greaterChildIdx = (treeIdx << 1) + 1;
			indexQueue.push(ivec3(greaterChildIdx, middleIdx + 1, endingIdx));
		}


		theTree[treeIdx - 1] = KDNode(
			balls[totalList[middleIdx]][axis],
			totalList[middleIdx],
			greaterChildIdx - 1,
			lesserChildIdx - 1
		);
	}



	//printf("max balls: %u, memfortree:%zu\n", numberOfPoints, memForTree);


	//printf("\n\n\n\n");
	//processNode(toReturn, balls, 0, numberOfPoints-1, totalList, 0, 1);

	//printf("\n\n\n\n");
	return theTree;
}




bool dbg = false;



vector<int32_t> getDotsInRange(const fvec4* balls, KDNode* theTree, uint32_t originDot, float range) {

	int maxLayer = 0;
	int numberVisited = 0;
	if(dbg)printf("\n\nstarting to find dots in range %.2f\n", range);
	const char* axises = "XYZ";
	bool hasNextLayer = true;


	queue<uint32_t> printQueue = queue<uint32_t>();

	printQueue.push(1);


	vector<int32_t> outputVector = vector<int32_t>();

	fvec4 theBall = balls[originDot];


	if (dbg)printf("origin: %s\n", glm::to_string(theBall).c_str());

	float rquared = range * range;

	while (!printQueue.empty()) {
		numberVisited++;
		uint32_t currIdx = printQueue.front();
		KDNode currNode = theTree[currIdx-1];
		printQueue.pop();


		int layer = int(glm::floor(glm::log2(float(currIdx))));
		maxLayer = glm::max(maxLayer, layer);
		int axis = layer % 3;

		float dsquared = distance2(fvec3(theBall.xyz), fvec3(balls[currNode.pointIdx].xyz));

		if ( (dsquared <= rquared) && (currNode.pointIdx != originDot)) {
			outputVector.push_back(currNode.pointIdx);
		}


		float val = theBall[axis];
		bool tooCloseToCall = glm::abs(val - currNode.value) <= range;
		bool possiblyGreater = val > currNode.value;
		bool possiblyLesser = val < currNode.value;


		if (currNode.lesserChild > 0 && (tooCloseToCall || possiblyLesser)) {
			printQueue.push(currNode.lesserChild+1);
		}
		if (currNode.greaterChild > 0 && (tooCloseToCall || possiblyGreater)) {
			printQueue.push(currNode.greaterChild+1);
		}
	}
	printf("max layer: %i, numVisitd: %i\n", maxLayer, numberVisited);
	return outputVector;
}

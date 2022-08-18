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





KDNode* makeKDTree(const Particle* balls, const size_t& numberOfPoints, const KDConstructionContext& kdConCon) {
	uint32_t* totalList = new uint32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i] = i;	
	}

	int cpuLayers = glm::ceil(glm::log2(float(numberOfPoints)) + 1)-2;
	//printf("cpu layers: %i\n", cpuLayers);

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(float(numberOfPoints))+1)))-1;
	
	//printf("memForTree: %zu\n", memForTree);
	
	KDNode* theTree = new KDNode[memForTree]();

	queue<ivec3> indexQueue = queue<ivec3>(); //where each vec3 is (treeIdx, staringIdx, endingIdx)
	
	indexQueue.push(ivec3(1, 0, numberOfPoints - 1));



	while (!indexQueue.empty() && indexQueue.size() < glm::pow(2, cpuLayers)) { //TODO: 64 = 2^6, nees to change this to just be a calculated value from the layer

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
			return balls[a].position[axis] < balls[b].position[axis];
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
			balls[totalList[middleIdx]].position[axis],
			totalList[middleIdx],
			greaterChildIdx - 1,
			lesserChildIdx - 1
		);
	}


	int numberOfStartingInputs = indexQueue.size();

	ivec3* tempInput = new ivec3[numberOfStartingInputs]();
	for (int f = 0; f < numberOfStartingInputs; f++) {
		tempInput[f] = indexQueue.front();
		//printf("%s\n", glm::to_string(tempInput[f]).c_str());
		indexQueue.pop();
	}



	/*printf("tempInput: %zu\n", numberOfStartingInputs);
	for (int f = 0; f < numberOfStartingInputs; f++) {
		printf("%i: %s\n", f, glm::to_string(tempInput[f]).c_str());
	}*/
	//cin.get();


	int totalLayers = glm::ceil(glm::log2(float(numberOfPoints)))+1;

	int numberOfEvents = 3;
	cl_event* writeEvents = new cl_event[numberOfEvents]();
	clEnqueueWriteBuffer(kdConCon.cmdQueue, kdConCon.clTotalList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, totalList, 0, NULL, &writeEvents[0]);
	clEnqueueWriteBuffer(kdConCon.cmdQueue, kdConCon.clTree, CL_FALSE, 0, sizeof(KDNode) * memForTree, theTree, 0, NULL, &writeEvents[1]);
	clEnqueueWriteBuffer(kdConCon.cmdQueue, kdConCon.clQA, CL_FALSE, 0, sizeof(ivec3)* numberOfStartingInputs, tempInput, 0, NULL, &writeEvents[2]);
	clWaitForEvents(numberOfEvents, writeEvents);

	/*printf("total list pre anything:\n");
	clEnqueueReadBuffer(kdConCon.cmdQueue, kdConCon.clTotalList, CL_TRUE, 0, sizeof(int32_t) * numberOfPoints, totalList, 0, NULL, NULL);

	for (int f = 0; f < numberOfPoints; f++) {
		printf("%i: %u: %s\n", f, totalList[f], glm::to_string(balls[totalList[f]]).c_str());
	}*/
	//cin.get();



	/*ivec3* tempOutput = new ivec3[numberOfStartingInputs]();
	clEnqueueReadBuffer(kdConCon.cmdQueue, kdConCon.clQA, CL_FALSE, 0, sizeof(ivec3) * numberOfStartingInputs, tempOutput, 0, NULL, &writeEvents[2]);
	printf("tempOutput %zu\n", numberOfStartingInputs);
	for (int f = 0; f < indexQueue.size(); f++) {
		printf("%i: %s\n", f, glm::to_string(tempOutput[f]).c_str());
	}*/
	//cin.get();










	cl_int status;

	status = clSetKernelArg(kdConCon.kdTreeKernel, 1, sizeof(cl_mem), &kdConCon.clTotalList);
	//printf("set kd kernel arg 1 status: %i\n", status);
	status = clSetKernelArg(kdConCon.kdTreeKernel, 2, sizeof(cl_mem), &kdConCon.clTree);
	//printf("set kd kernel arg 2 status: %i\n", status);


	int smolNumberOfPoints = int(numberOfPoints);

	status = clSetKernelArg(kdConCon.kdTreeKernel, 5, sizeof(int), &smolNumberOfPoints);
	//printf("set kd kernel arg 5 status: %i\n", status);


	cl_event* kernelEvent = new cl_event();
	//printf("total layer: %i\n", totalLayers);

	for (int i = 0; i < totalLayers - cpuLayers; i++) {
		
		status = clSetKernelArg(kdConCon.kdTreeKernel, 3+(i%2), sizeof(cl_mem), &kdConCon.clQA);//qa is input on even, output on odd, (kernel args 3 and 4)
		status = clSetKernelArg(kdConCon.kdTreeKernel, 3+((i+1)%2), sizeof(cl_mem), &kdConCon.clQB);


		size_t globalWorkSize[3] = { glm::pow(2, i+cpuLayers), 1, 1 };




		//enqueue a sorting kernel for each input to sort it

		clFinish(kdConCon.cmdQueue);
		status = clEnqueueNDRangeKernel(kdConCon.cmdQueue, kdConCon.kdTreeKernel, 1, NULL, globalWorkSize, NULL, 0, NULL, kernelEvent);
		clWaitForEvents(1, kernelEvent);

		
	}
	clFinish(kdConCon.cmdQueue);




	clEnqueueReadBuffer(kdConCon.cmdQueue, kdConCon.clTree, CL_TRUE, 0, sizeof(KDNode) * memForTree, theTree, 1, kernelEvent, NULL);






	//for (int i = 0; i < memForTree; i++) {
	//	KDNode x = theTree[i];
	//	printf("%i, %f, %i, %i, %i\n",i, x.value, x.pointIdx, x.greaterChild, x.lesserChild);
	//}

	//for (auto f : indexQueue._Get_container()) {
	//	printf("%s\n", glm::to_string(f).c_str());
	//}


	//queue<ivec3> tempQueue = queue<ivec3>(indexQueue);
	//printf("tempQueue.size: %zu\n", tempQueue.size());
	//while (!tempQueue.empty()) {
	//	printf("%s\n", glm::to_string(tempQueue.front()).c_str());
	//	tempQueue.pop();
	//}
	//exit(0);





	//printf("max balls: %u, memfortree:%zu\n", numberOfPoints, memForTree);


	//printf("\n\n\n\n");
	//processNode(toReturn, balls, 0, numberOfPoints-1, totalList, 0, 1);

	//printf("\n\n\n\n");
	return theTree;
}




bool dbg = false;



vector<int32_t> getDotsInRange(const Particle* particles, KDNode* theTree, uint32_t originDot, float range) {

	int maxLayer = 0;
	int numberVisited = 0;
	if(dbg)printf("\n\nstarting to find dots in range %.2f\n", range);
	const char* axises = "XYZ";
	bool hasNextLayer = true;


	queue<uint32_t> printQueue = queue<uint32_t>();

	printQueue.push(1);


	vector<int32_t> outputVector = vector<int32_t>();

	fvec3 theBall = particles[originDot].position;


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

		float dsquared = distance2(theBall, particles[currNode.pointIdx].position);

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
	//printf("max layer: %i, numVisitd: %i\n", maxLayer, numberVisited);
	return outputVector;
}

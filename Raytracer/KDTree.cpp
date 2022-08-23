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





void makeKDTree(const Particle* particles, const size_t& numberOfPoints, const KDConstructionContext& kdConCon) {


	cl_int status; 
	
	uint32_t* totalList = new uint32_t[numberOfPoints];
	int32_t* nodeIndexList = new int32_t[numberOfPoints];
	for (uint32_t i = 0; i < numberOfPoints; i++) {
		totalList[i] = i;
		nodeIndexList[i] = 1;
	}

	int cpuLayers = 1;// glm::ceil(glm::log2(float(numberOfPoints)) + 1) - 2;

	//printf("cpu layers: %i\n", cpuLayers);

	size_t memForTree = size_t(glm::pow(2, glm::ceil(glm::log2(float(numberOfPoints))+1)))-1;
	
	//printf("memForTree: %zu\n", memForTree);
	
	KDNode* theTree = new KDNode[memForTree]();

	queue<ivec4> indexQueue = queue<ivec4>(); //where each vec3 is (treeIdx, staringIdx, endingIdx)
	
	indexQueue.push(ivec4(1, 0, numberOfPoints - 1, 0));

	


	while (!indexQueue.empty() && indexQueue.size() < glm::pow(2, cpuLayers)) { //TODO: 64 = 2^6, nees to change this to just be a calculated value from the layer

		ivec4 curr = indexQueue.front();
		indexQueue.pop();

		int treeIdx = curr.x;
		int startingIdx = curr.y;
		int endingIdx = curr.z;
		int middleIdx = (startingIdx + endingIdx) / 2;

		int layer = int(glm::floor(glm::log2(float(treeIdx))));
		int axis = layer % 3;

		
		int lesserChildIdx = -1;
		int greaterChildIdx = -1;

		auto mySert = [particles, axis](size_t a, size_t b) {
			return particles[a].position[axis] < particles[b].position[axis];
		};

		sort(totalList + startingIdx, totalList + endingIdx + 1, mySert);

		if (middleIdx > startingIdx) {//has lesser children
			lesserChildIdx = treeIdx << 1;
			indexQueue.push(ivec4(lesserChildIdx, startingIdx, middleIdx - 1, 0));
		}
		if (middleIdx < endingIdx) { //has greater children
			greaterChildIdx = (treeIdx << 1) + 1;
			indexQueue.push(ivec4(greaterChildIdx, middleIdx + 1, endingIdx, 0));
		}


		theTree[treeIdx - 1] = KDNode(
			particles[totalList[middleIdx]].position[axis],
			totalList[middleIdx],
			greaterChildIdx - 1,
			lesserChildIdx - 1
		);
	} 


	
	int firstGPULayer = cpuLayers;
	
	int startingNode = int(glm::pow(2, firstGPULayer)) - 1;

	int numberOfStartingInputs = 1;

	ivec4* tempInput = new ivec4[numberOfStartingInputs]();
	tempInput[0] = ivec4(1, 0, numberOfPoints - 1, 0);

	//int i = 0;
	//for (int f = 0; f < numberOfStartingInputs; f++) {
	//	tempInput[f] = indexQueue.front();
	//	//printf("tempInput: %i, %i, %i\n", tempInput[f].x, tempInput[f].y, tempInput[f].z);
	//	for (int j = 0; j < (tempInput[f].z - tempInput[f].y) + 1; j++) {
	//		nodeIndexList[i++] = startingNode;
	//	}
	//	startingNode++;	
	//	nodeIndexList[i++] = 0;
	//	indexQueue.pop();
	//}



	/*printf("tempInput: %zu\n", numberOfStartingInputs);
	for (int f = 0; f < numberOfStartingInputs; f++) {
		printf("%i: %s\n", f, glm::to_string(tempInput[f]).c_str());
	}*/
	//cin.get();


	int totalLayers = glm::ceil(glm::log2(float(numberOfPoints)))+1;

	cl_event* writeEvents = new cl_event[4]();
	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clTotalList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, totalList, 0, NULL, &writeEvents[0]);
	if (status)printf("kd write 1 %i\n", status);

	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clnodeIndexList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, nodeIndexList, 0, NULL, &writeEvents[1]);
	if (status)printf("bitonic write 1 %i\n", status);

	//printf("\n%p, %p, %i, %p, %p\n\n", &kdConCon.cmdQueue, &kdConCon.clTheTree, memForTree, theTree, &writeEvents[1]);


	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clTheTree, CL_FALSE, 0, sizeof(KDNode) * memForTree, theTree, 0, NULL, &writeEvents[2]);
	if (status)printf("kd write 2 %i\n", status);

	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clQA, CL_FALSE, 0, sizeof(ivec4)* numberOfStartingInputs, tempInput, 0, NULL, &writeEvents[3]);
	if (status)printf("kd write 3 %i\n", status);
	clWaitForEvents(4, writeEvents);

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










	status = clSetKernelArg(*kdConCon.kdTreeKernel, 1, sizeof(cl_mem), kdConCon.clTotalList);
	if (status)printf("kd kernel 1 %i\n", status);
	//printf("set kd kernel arg 1 status: %i\n", status);
	//status = clSetKernelArg(kdConCon.kdTreeKernel, 2, sizeof(cl_mem), &kdConCon.clTree);
	//printf("set kd kernel arg 2 status: %i\n", status);


	int smolNumberOfPoints = int(numberOfPoints);

	status = clSetKernelArg(*kdConCon.kdTreeKernel, 5, sizeof(int), &smolNumberOfPoints);
	if (status)printf("kd kernel 5 %i\n", status);
	//printf("set kd kernel arg 5 status: %i\n", status);

	//printf("total layers: %i \n", totalLayers);
	cl_event* kernelEvent = new cl_event();
	//printf("total layer: %i\n", totalLayers);

	/*printf("node index list:\n");
	for (int i = 0; i < numberOfPoints; i++) {
		printf("%i, ", totalList[i]);
	}
	printf("\n");
	printf("node index list:\n");
	for (int i = 0; i < numberOfPoints; i++) {
		printf("%i, ", nodeIndexList[i]);
	}
	printf("\n");*/


	for (int layer = 0; layer < totalLayers; layer++) {
		

		printf("\n\nlayer %i\n", layer);
		int axis = layer % 3;

		size_t bitonicGlobalWorkSize[3] = { (numberOfPoints+1)/2, 1, 1 };

		status = clSetKernelArg(*kdConCon.bitonicKernel, 3, sizeof(int), &axis);
		if (status)printf("bitonic kernel 0 %i\n", status);

		for (uint i = 0; (uint(1) << i) < numberOfPoints; i += 1) {
			for (uint j = 0; j <= i; j += 1) {

				int three = i - j;
				int four = int(j == 0);

				status = clSetKernelArg(*kdConCon.bitonicKernel, 4, sizeof(int), &three);
				if (status)printf("bitonic kernel 0 %i\n", status);
				status = clSetKernelArg(*kdConCon.bitonicKernel, 5, sizeof(int), &four);
				if (status)printf("bitonic kernel 0 %i\n", status);

				status = clEnqueueNDRangeKernel(*kdConCon.cmdQueue, *kdConCon.bitonicKernel, 1, NULL, bitonicGlobalWorkSize, NULL, 0, NULL, kernelEvent);
				if (status)printf("bitonic execution %i, %i\n", layer, status);
				clWaitForEvents(1, kernelEvent);
			}
		}


		clFinish(*kdConCon.cmdQueue);

		/*status = clEnqueueReadBuffer(*kdConCon.cmdQueue, *kdConCon.clTotalList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, totalList, 0, NULL, NULL);
		if (status)printf("kd read 5 %i\n", status);
		status = clEnqueueReadBuffer(*kdConCon.cmdQueue, *kdConCon.clnodeIndexList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, nodeIndexList, 0, NULL, NULL);
		if (status)printf("bitonic write 1 %i\n", status);
		clFinish(*kdConCon.cmdQueue);

		for (int fda = 0; fda < numberOfPoints; fda++) {
			int jk = totalList[fda];
			if (fda == numberOfPoints / 2)printf("\nhalf\n\n");
			printf("\t%i\t%i(%.2f, %.2f, %.2f)\n", jk, nodeIndexList[jk], particles[jk].position.x, particles[jk].position.y, particles[jk].position.z);
		}
		printf("\n");
		printf("sorted\n");
		cin.get();*/

		status = clSetKernelArg(*kdConCon.kdTreeKernel, 3 + (layer%2), sizeof(cl_mem), kdConCon.clQA);//qa is input on even, output on odd, (kernel args 3 and 4)
		status = clSetKernelArg(*kdConCon.kdTreeKernel, 3 + ((layer+1)%2), sizeof(cl_mem), kdConCon.clQB);

		status = clSetKernelArg(*kdConCon.kdTreeKernel, 7, sizeof(int), &layer);
		if (status)printf("kdConCon 7 %i\n", status);
		status = clSetKernelArg(*kdConCon.kdTreeKernel, 8, sizeof(int), &axis);
		if (status)printf("kdConCon 8 %i\n", status);



		size_t kdGlobalWorkSize[3] = { glm::pow(2, layer), 1, 1 };

		status = clEnqueueNDRangeKernel(*kdConCon.cmdQueue, *kdConCon.kdTreeKernel, 1, NULL, kdGlobalWorkSize, NULL, 0, NULL, kernelEvent);
		if (status)printf("kd kernel execution %i,  %i\n",layer, status);
		clWaitForEvents(1, kernelEvent);
		clFinish(*kdConCon.cmdQueue);

		
	}
	clFinish(*kdConCon.cmdQueue);




	//status = clEnqueueReadBuffer(*kdConCon.cmdQueue, *kdConCon.clTheTree, CL_TRUE, 0, sizeof(KDNode) * memForTree, theTree, 1, kernelEvent, NULL);
	//if (status)printf("kd read 1 %i\n", status);


	//printTree(theTree);



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
	//return theTree;
	//printf("kd clmem: %p\n", &kdConCon.clTheTree);
}




bool dbg = false;



//vector<int32_t> getDotsInRangeOld(const Particle* particles, KDNode* theTree, uint32_t originDot, float range) {
//
//	int maxLayer = 0;
//	int numberVisited = 0;
//	if(dbg)printf("\n\nstarting to find dots in range %.2f\n", range);
//	const char* axises = "XYZ";
//	bool hasNextLayer = true;
//
//
//	queue<uint32_t> printQueue = queue<uint32_t>();
//
//	printQueue.push(1);
//
//
//	vector<int32_t> outputVector = vector<int32_t>();
//
//	fvec3 theBall = particles[originDot].position.xyz;
//
//
//	if (dbg)printf("origin: %s\n", glm::to_string(theBall).c_str());
//
//	float rsquared = range * range;
//	while (!printQueue.empty()) {
//		numberVisited++;
//
//		uint32_t currIdx = printQueue.front();
//		printf("visiting tree index %u\n", currIdx);
//		KDNode currNode = theTree[currIdx-1];
//
//		printQueue.pop();
//
//
//		int layer = int(glm::floor(glm::log2(float(currIdx))));
//		maxLayer = glm::max(maxLayer, layer);
//		int axis = layer % 3;
//
//		float dsquared = distance2(theBall, fvec3(particles[currNode.pointIdx].position));
//
//		if ( (dsquared <= rsquared) && (currNode.pointIdx != originDot)) {
//			outputVector.push_back(currNode.pointIdx);
//		}
//
//
//		float val = theBall[axis];
//		bool tooCloseToCall = glm::abs(val - currNode.value) <= range;
//		bool possiblyGreater = val > currNode.value;
//		bool possiblyLesser = val < currNode.value;
//
//
//		if (currNode.lesserChild > 0 && (tooCloseToCall || possiblyLesser)) {
//			printQueue.push(currNode.lesserChild+1);
//		}
//		if (currNode.greaterChild > 0 && (tooCloseToCall || possiblyGreater)) {
//			printQueue.push(currNode.greaterChild+1);
//		}
//	}
//	//printf("max layer: %i, numVisitd: %i\n", maxLayer, numberVisited);
//	return outputVector;
//}

string toBinary(int n, int len)
{
	string binary;
	for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
		binary += (n & i) ? "1" : "0";
	}
	return binary;
}

vector<int32_t> getDotsInRange(const Particle* particles, KDNode* theTree, uint32_t originDot, float range) {

	vector<int32_t> toReturn = vector<int32_t>();

	int currIdx = 1;
	uint lesserTraversals = 0;
	uint greaterTraversals = 0;

	int traveledNodes = 0;

	// for(int i = 0; i < 60; i++) {
	//     struct KDNode c = theTree[i];
	//     printf("i: %i, idx: %u, V: %f, L: %i, R: %i\n", i, c.pointIdx, c.value, c.greaterChild, c.lesserChild);
	// }

	// return;
	fvec3 theBall = particles[originDot].position;

	float rsquared = range * range;

	bool lastParent = false;

	printf("cpu version:\n");
	while (currIdx > 0) { // or while(true)???
		//printf("traveledNodes: %i\n", traveledNodes);
		//traveledNodes++;
		//printf("\n=======================\nvisiting tree index %u\n", currIdx);
		struct KDNode currNode = theTree[currIdx - 1];

		printf("\nvisiting tree index %u\n", currIdx-1);

		int layer = (int)(floor(log2((float)(currIdx))));
		int axis = layer % 3;
		float dsquared = glm::distance2(theBall, vec3(particles[currNode.pointIdx].position));
		if ((dsquared <= rsquared) && (currNode.pointIdx != originDot) && (!lastParent)) {
			toReturn.push_back(currNode.pointIdx);
			printf("adding %i\n", currNode.pointIdx);
		}


		float val = theBall[axis];
		bool tooCloseToCall = fabs(val - currNode.value) <= range;


		printf("%f - %f = %f <= %f\n", val, currNode.value, fabs(val - currNode.value), range);
		printf("too close to call %i\n", int(tooCloseToCall));

		bool possiblyGreater = (tooCloseToCall || (val > currNode.value)) && (currNode.greaterChild >= 0);
		bool possiblyLesser = (tooCloseToCall || (val < currNode.value)) && (currNode.lesserChild >= 0);
		//printf("%f, %f\n",val, currNode.value);
		//printf("%u, %u, %i, %u\n", lesserTraversals, greaterTraversals, possiblyGreater, possiblyLesser);

		/*int goingLesser = possiblyLesser && !(1 & lesserTraversals);
		int goingGreater = !goingLesser && possiblyGreater && !(1 & greaterTraversals);
		int goingParent = !goingLesser && !goingGreater;

		lesserTraversals = lesserTraversals << (goingGreater || goingLesser);
		greaterTraversals = greaterTraversals << (goingGreater || goingLesser);
		lesserTraversals = lesserTraversals >> goingParent;
		greaterTraversals = greaterTraversals >> goingParent;

		currIdx = (goingLesser * (currNode.lesserChild + 1)) +
			(goingGreater * (currNode.greaterChild + 1)) +
			(goingParent * (currIdx >> 1));*/
		//printf("%i, %i, %i, %u\n", goingLesser, goingGreater, goingParent, currIdx);
		lastParent = false;
		 if(possiblyLesser && !(1&lesserTraversals)) {
			 
			 printf("going lesser\n");
		     lesserTraversals |= 1;
		     lesserTraversals = lesserTraversals<<1;
		     greaterTraversals = greaterTraversals<<1;
		     currIdx = currNode.lesserChild+1;
		 }
		 else {
			 if (possiblyGreater && !(1 & greaterTraversals)) {
				 printf("going greater\n");
				 greaterTraversals |= 1;
				 lesserTraversals = lesserTraversals << 1;
				 greaterTraversals = greaterTraversals << 1;
				 currIdx = currNode.greaterChild + 1;
			 }
			 else {
				 lastParent = true;
				 printf("going parent\n");
				 currIdx = currIdx / 2;
				 lesserTraversals = lesserTraversals >> 1;
				 greaterTraversals = greaterTraversals >> 1;
			 }
		 }
		 printf("greaterChild: %i, lesserChild: %i\n", currNode.greaterChild, currNode.lesserChild);
		 printf("pL: %i, pG: %i\n", int(possiblyLesser), int(possiblyGreater));
		 printf("origin index: %u, this nodes index: %i\n", originDot, currIdx - 1);
		 printf("lT: %u, gT: %u\n", lesserTraversals, greaterTraversals);
		 printf("lT: %s\n", toBinary(lesserTraversals, 32).c_str());
		 printf("gT: %s\n", toBinary(greaterTraversals, 32).c_str());


	}
	return toReturn;
}

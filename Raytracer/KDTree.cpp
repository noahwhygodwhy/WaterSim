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


	int numberOfStartingInputs = 1;
	ivec4 tempInput(1, 0, numberOfPoints - 1, 0);

	int totalLayers = glm::ceil(glm::log2(float(numberOfPoints)))+1;



//write initial arrays to gpu
	cl_event* writeEvents = new cl_event[3]();
	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clTotalList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, totalList, 0, NULL, &writeEvents[0]);
	if (status)printf("kd write 1 %i\n", status);
	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clnodeIndexList, CL_FALSE, 0, sizeof(int32_t) * numberOfPoints, nodeIndexList, 0, NULL, &writeEvents[1]);
	if (status)printf("bitonic write 1 %i\n", status);
	status = clEnqueueWriteBuffer(*kdConCon.cmdQueue, *kdConCon.clQA, CL_FALSE, 0, sizeof(ivec4)* numberOfStartingInputs, &tempInput, 0, NULL, &writeEvents[2]);
	if (status)printf("kd write 3 %i\n", status);
	clWaitForEvents(3, writeEvents);


	
	

	//gotta convert it to a 32 

	cl_event* kernelEvent = new cl_event();
	
	for (int layer = 0; layer < totalLayers; layer++) {

///////do the sort
		int axis = layer % 3;

		size_t bitonicGlobalWorkSize[3] = { (numberOfPoints)/2, 1, 1 };

		status = clSetKernelArg(*kdConCon.bitonicKernel, 3, sizeof(int), &axis);
		if (status)printf("bitonic kernel 0 %i\n", status);

		for (uint i = 0; (uint(1) << i) < numberOfPoints; i += 1) {
			for (uint j = 0; j <= i; j += 1) {

				int three = i - j;
				int four = int(j == 0);
				int five = numberOfPoints;
				status = clSetKernelArg(*kdConCon.bitonicKernel, 4, sizeof(int), &three);
				if (status)printf("bitonic kernel 0 %i\n", status);
				status = clSetKernelArg(*kdConCon.bitonicKernel, 5, sizeof(int), &four);
				if (status)printf("bitonic kernel 0 %i\n", status);
				status = clSetKernelArg(*kdConCon.bitonicKernel, 6, sizeof(int), &five);
				if (status)printf("bitonic kernel 0 %i\n", status);

				// printf("enqueueing kernel %u, %u\n", i, j);
				status = clEnqueueNDRangeKernel(*kdConCon.cmdQueue, *kdConCon.bitonicKernel, 1, NULL, bitonicGlobalWorkSize, NULL, 0, NULL, kernelEvent);
				if (status)printf("bitonic execution %i, %i\n", layer, status);
				clWaitForEvents(1, kernelEvent);
			}
		}

		clFinish(*kdConCon.cmdQueue);


//////assign nodes and split into groups
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
}




bool dbg = false;


string toBinary(int n, int len)
{
	string binary;
	for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
		binary += (n & i) ? "1" : "0";
	}
	return binary;
}


//unused cause now it's on the gpu
vector<int32_t> getDotsInRange(const Particle* particles, KDNode* theTree, uint32_t originDot, float range) {

	vector<int32_t> toReturn = vector<int32_t>();

	int currIdx = 1;
	uint lesserTraversals = 0;
	uint greaterTraversals = 0;

	int traveledNodes = 0;

	fvec3 theBall = particles[originDot].position;

	float rsquared = range * range;

	bool lastParent = false;

	while (currIdx > 0) { // or while(true)???
		//printf("traveledNodes: %i\n", traveledNodes);
		//traveledNodes++;
		//printf("\n=======================\nvisiting tree index %u\n", currIdx);
		struct KDNode currNode = theTree[currIdx - 1];

		//printf("\nvisiting tree index %u\n", currIdx-1);

		int layer = (int)(floor(log2((float)(currIdx))));
		int axis = layer % 3;
		float dsquared = glm::distance2(theBall, vec3(particles[currNode.pointIdx].position));
		if ((dsquared <= rsquared) && (currNode.pointIdx != originDot) && (!lastParent)) {
			toReturn.push_back(currNode.pointIdx);
			//printf("adding %i\n", currNode.pointIdx);
		}


		float val = theBall[axis];
		bool tooCloseToCall = fabs(val - currNode.value) <= range;


		/*printf("%f - %f = %f <= %f\n", val, currNode.value, fabs(val - currNode.value), range);
		printf("too close to call %i\n", int(tooCloseToCall));*/

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
			 
			// printf("going lesser\n");
		     lesserTraversals |= 1;
		     lesserTraversals = lesserTraversals<<1;
		     greaterTraversals = greaterTraversals<<1;
		     currIdx = currNode.lesserChild+1;
		 }
		 else {
			 if (possiblyGreater && !(1 & greaterTraversals)) {
				 //printf("going greater\n");
				 greaterTraversals |= 1;
				 lesserTraversals = lesserTraversals << 1;
				 greaterTraversals = greaterTraversals << 1;
				 currIdx = currNode.greaterChild + 1;
			 }
			 else {
				 lastParent = true;
				 //printf("going parent\n");
				 currIdx = currIdx / 2;
				 lesserTraversals = lesserTraversals >> 1;
				 greaterTraversals = greaterTraversals >> 1;
			 }
		 }
		 /*printf("greaterChild: %i, lesserChild: %i\n", currNode.greaterChild, currNode.lesserChild);
		 printf("pL: %i, pG: %i\n", int(possiblyLesser), int(possiblyGreater));
		 printf("origin index: %u, this nodes index: %i\n", originDot, currIdx - 1);
		 printf("lT: %u, gT: %u\n", lesserTraversals, greaterTraversals);
		 printf("lT: %s\n", toBinary(lesserTraversals, 32).c_str());
		 printf("gT: %s\n", toBinary(greaterTraversals, 32).c_str());*/


	}
	return toReturn;
}

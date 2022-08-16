#include "rand.cl"
#include "Shared.cl"
//#include "QuickSort.cl"



__kernel void watersim(
    __global float3* positions,
    __global float3* velocities
    )  {
    int idx = get_global_id(0);    
}



float indexF3(float3 a, int index) {
    switch(index) {
        case 0: return a.x;
        case 1: return a.y;
        default: return a.z;
    }
}



void gSwap(__global unsigned int* a, __global unsigned int* b) {
    unsigned int t = *a;
    *a = *b;
    *b = t;
}





//https://en.wikipedia.org/wiki/Bitonic_sorter
void bitonicSort(
    int start,
    int end,
    int axis,
    __global float3* positions,
    __global unsigned int* totalList
) {
    //bool dbg = (get_global_id(0)==0);
    
    //if(dbg)printf("start: %i, end: %i, n: %i\n", start, end, (start-end)+1);

    int n = (end-start)+1;



    for (int k = 2; k <= n; k *= 2){ // k is doubled every iteration 
        for (int j = k/2; j > 0; j /= 2) { // j is halved at every iteration, with truncation of fractional parts
            for (int i = 0; i < n; i++) {
                int l = i^j; // in C-like languages this is "i ^ j"

                //printf("idx: %i, l: %i, i: %i, k: %i", get_global_id(0), l, i, k);

                if (l > i){
                    if (!(((i & k) == 0) ^ (indexF3(positions[totalList[i]], axis) > indexF3(positions[totalList[l]], axis))))
                    {
                        gSwap(totalList + i, totalList + l );
                    }
                }
            }
        }
    }
}


__kernel void makeKDTree(
    __global float3* positions,
    __global unsigned int* totalList,
    __global struct KDNode* theTree,
    __global int3* input,
    __global int3* output,
    int inputCount
){
    int idx = get_global_id(0);    

    int3 curr = input[idx];
    int treeIdx = curr.x;
	int startingIdx = curr.y;
	int endingIdx = curr.z;
	int middleIdx = (startingIdx + endingIdx) / 2;
	int layer = (int)(floor(log2((float)(treeIdx))));
	int axis = layer % 3;
	int lesserChildIdx = -1;
	int greaterChildIdx = -1;

    bitonicSort(startingIdx, endingIdx, axis, positions, totalList);

    int hasGreater = (int)(middleIdx < endingIdx);
    int noGreater = 1-hasGreater;
    int hasLesser = (int)(middleIdx > startingIdx);
    int noLesser = 1-hasLesser;

    lesserChildIdx = ((treeIdx << 1)*hasLesser)+(noLesser*-1);
	output[idx*2] = ((int3)(lesserChildIdx, startingIdx, middleIdx - 1));

    
	output[(idx*2)+1] = ((int3)(greaterChildIdx, middleIdx + 1, endingIdx));
    
    lesserChildIdx = (((treeIdx << 1) + 1)*hasGreater)+(noGreater*-1);

  


    struct KDNode nodeToAdd = {
		indexF3(positions[totalList[middleIdx]], axis),
		totalList[middleIdx],
		greaterChildIdx - 1,
		lesserChildIdx - 1
    };
	theTree[treeIdx - 1] = nodeToAdd;
}



//slide 25 https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/opencl.2pp.pdf
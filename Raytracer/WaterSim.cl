#include "Shared.cl"
#include "rand.cl"
//#include "QuickSort.cl"

// TODO: https://www.diva-portal.org/smash/get/diva2:703754/FULLTEXT01.pdf

// page 29

__constant float NEIGHBOR_RADIUS = 3.0f;
__constant float NEIGHBOR_RADIUS_SQUARED = 9.0f;

float indexF3(float3 a, int index) {
    switch (index) {
    case 0:
        return a.x;
    case 1:
        return a.y;
    default:
        return a.z;
    }
}
float indexF4(__global float4* a, int index) {
    switch (index) {
    case 0:
        return a->x;
    case 1:
        return a->y;
    case 2:
        return a->z;
    default:
        return a->w;
  }
}

float distance_squared(float3 a, float3 b) {
    float3 c = a - b;
    return (c.x * c.x) + (c.y + c.y) + (c.z * c.z);
}

void gSwap(__global unsigned int *a, __global unsigned int *b) {
  unsigned int t = *a;
  *a = *b;
  *b = t;
}

uint nextPowerOfTwo(uint x) {
    x -= 1;
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;
    x += 1;

    return x;
}



//this bitonic sort was essentially written by riley broderick
//I tried but god damn I could not get it to function
__kernel void bitonicSort(
    __global struct Particle *particles,
    __global unsigned int *totalList,
    __global unsigned int *indexOfNodeList,
    int axis,
    int blockWidth,
    int reflect
){

    size_t numParticles = get_global_size(0);
    //int idx = get_global_id(0);

    uint mask = ((uint)(1) << blockWidth) - 1;
    uint offset = (uint)(1) << blockWidth;
    uint limit = nextPowerOfTwo(numParticles) / 2;

    for (
            uint i = get_local_id(0) + (get_local_size(0)*get_group_id(0));
            i < limit;
            i += get_local_size(0) * get_num_groups(0)
    ) {
        uint j = ((i & ~mask) << 1) + (i & mask);
        uint k = ((i & ~mask) << 1) + ((reflect ? ~i : i) & mask) + offset;

        if (k < numParticles) {
            
            float jVal = indexF4(&particles[totalList[j]].position, axis);
            float kVal = indexF4(&particles[totalList[k]].position, axis);
            if (jVal > kVal) {
                
                int swapSpace = totalList[j];
                totalList[j]= totalList[k];
                totalList[k] = swapSpace;
            }
        }
    }

    // int l = idx ^ j; 
    // if ((indexOfNodeList[idx] == indexOfNodeList[l]) && (l > idx))  
    // {
    //     float idxVal = indexF4(&particles[totalList[idx]].position, axis);
    //     float lVal = indexF4(&particles[totalList[l]].position, axis);

    //     if((idx&k)==0){
    //         if(idxVal > lVal) {
    //             int swapSpace = totalList[idx];
    //             totalList[idx]= totalList[l];
    //             totalList[l] = swapSpace;
    //         }
    //     } else {
    //         if(idxVal < lVal) {
    //             int swapSpace = totalList[idx];
    //             totalList[idx]= totalList[l];
    //             totalList[l] = swapSpace;
    //         }
    //     }
    // }
            
}

__kernel void computeDP(
    __global struct Particle *particles,
    __global struct KDNode *theTree, 
    float deltaTime) {

    int idx = get_global_id(0);
    //particles[idx].velocity = (float4)(1.0, 1.0, 0.0, 0.0);
    //return;

    particles[idx].velocity = (float4)(0.0, 0.0, 1.0, 0.0);

    if (idx != 2) {
        return;
    }
    particles[idx].velocity = (float4)(1.0, 0.0, 0.0, 0.0);
    uint currIdx = 1;
    float3 theBall = particles[idx].position.xyz;

    // you can backtrack cause its stored implicitly, you can math the parent
    // node, and choose to ignore children

    uint lesserTraversals = 0;
    uint greaterTraversals = 0;

    int traveledNodes = 0;

    // for(int i = 0; i < 60; i++) {
    //     struct KDNode c = theTree[i];
    //     printf("i: %i, idx: %u, V: %f, L: %i, R: %i\n", i, c.pointIdx, c.value, c.greaterChild, c.lesserChild);
    // }

    // return;

    bool lastParent = false;
    
	//printf("gpu version:\n");
    printf("gpu:\n");
    while (currIdx > 0) { // or while(true)???
        //printf("currIdx: %i\n", currIdx);
        //printf("traveledNodes: %i\n", traveledNodes);
        //traveledNodes++;
        //printf("visiting tree index %u\n", currIdx);
        struct KDNode currNode = theTree[currIdx - 1];

		//printf("visiting tree index %u\n", currIdx-1);

        int layer = (int)(floor(log2((float)(currIdx))));
        int axis = layer % 3;
        float dsquared = distance_squared(theBall, particles[currNode.pointIdx].position.xyz);
        if ((dsquared <= NEIGHBOR_RADIUS_SQUARED) && (currNode.pointIdx != idx) && !lastParent) {
            float distance = sqrt(dsquared);
            printf("%i, ", currNode.pointIdx);
            particles[currNode.pointIdx].velocity = (float4)(1.0, 1.0, 0.0, 0.0);// * (distance/NEIGHBOR_RADIUS); // temp
        // DO CALCULATIONS INVOLVING NEIGHBORS HERE
        }


        float val = indexF3(theBall, axis);
        int tooCloseToCall = fabs(val - currNode.value) <= NEIGHBOR_RADIUS;

        int possiblyGreater = (tooCloseToCall || (val > currNode.value)) && currNode.greaterChild >= 0;
        int possiblyLesser = (tooCloseToCall || (val < currNode.value)) && currNode.lesserChild >= 0;
        //printf("%f, %f\n",val, currNode.value);
        //printf("%u, %u, %i, %u\n", lesserTraversals, greaterTraversals, possiblyGreater, possiblyLesser);

        lastParent = false;
        if(possiblyLesser && !(1&lesserTraversals)) {
              
              //printf("going lesser\n");
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




        // int goingLesser = possiblyLesser && !(1 & lesserTraversals);
        // int goingGreater = !goingLesser && possiblyGreater && !(1 & greaterTraversals);
        // int goingParent = !goingLesser && !goingGreater;

        // lesserTraversals = lesserTraversals << (goingGreater || goingLesser);
        // greaterTraversals = greaterTraversals << (goingGreater || goingLesser);
        // lesserTraversals = lesserTraversals >> goingParent;
        // greaterTraversals = greaterTraversals >> goingParent;

        // currIdx = (goingLesser * (currNode.lesserChild+1)) +
        //         (goingGreater * (currNode.greaterChild+1)) +
        //         (goingParent * (currIdx >> 1));


        //         // 
        //printf("%i, %i, %i, %u\n", goingLesser, goingGreater, goingParent, currIdx);

        // if(possiblyLesser && !(1&lesserTraversals)) {
        //     lesserTraversals &= 1;
        //     lesserTraversals = lesserTraversals<<1;
        //     greaterTraversals = greaterTraversals<<1;
        //     currIdx = currNode.lesserChild;
        // }
        // else
        // if(possiblyGreater && !(1&greaterTraversals)) {
        //     greaterTraversals &= 1;
        //     lesserTraversals = lesserTraversals<<1;
        //     greaterTraversals = greaterTraversals<<1;
        //     currIdx = currNode.greaterChild;
        // }
        // else {
        //     currIdx = currIdx/2;
        //     lesserTraversals = lesserTraversals>>1;
        //     greaterTraversals = greaterTraversals>>1;
        // }
    }
    printf("\n");
  // positions[idx] = positions[idx]+(velocities[idx]*deltaTime);
}

// __kernel void compute(
//     __global float3* positions,
//     //__global float3* velocities,
//     __global float* densities,
//     __global float* pressures,
//     __global struct KDNode* theTree,
//     float deltaTime
//     )  {
//     int idx = get_global_id(0);

//     positions[idx] = positions[idx]+(velocities[idx]*deltaTime);
// }


// https://en.wikipedia.org/wiki/Bitonic_sorter
// void bitonicSort(int start, int end, int axis,
//                  __global struct Particle *particles,
//                  __global unsigned int *totalList) {
//   // bool dbg = (get_global_id(0)==0);

//   // if(dbg)printf("start: %i, end: %i, n: %i\n", start, end, (start-end)+1);

//   int n = (end - start) + 1;

//   for (int k = 2; k <= n; k *= 2) {      // k is doubled every iteration
//     for (int j = k / 2; j > 0; j /= 2) { // j is halved at every iteration, with
//                                          // truncation of fractional parts
//       for (int i = 0; i < n; i++) {
//         int l = i ^ j; // in C-like languages this is "i ^ j"

//         // printf("idx: %i, l: %i, i: %i, k: %i", get_global_id(0), l, i, k);

//         if (l > i) {
//           if (!(((i & k) == 0) ^
//                 (indexF4(&particles[totalList[i]].position, axis) >
//                  indexF4(&particles[totalList[l]].position, axis)))) {
//             gSwap(totalList + i, totalList + l);
//           }
//         }
//       }
//     }
//   }
// }

__kernel void makeKDTree(__global struct Particle *particles,
                         __global unsigned int *totalList,
                         __global struct KDNode *theTree, 
                         __global int4 *input,
                         __global int4 *output, 
                         int inputCount,
                         __global int *nodeIndexList,
                         int layer,
                         int axis) {
    int idx = get_global_id(0);

    int4 curr = input[idx];
    int treeIdx = curr.x;
    if(treeIdx < 0)
    {
        output[idx * 2] = ((int4)(-1, 0,0, 0));
        output[(idx * 2) + 1] = ((int4)(-1, 0,0, 0));
        return;
    }
    int startingIdx = curr.y;
    int endingIdx = curr.z;
    int middleIdx = (startingIdx + endingIdx) / 2;
    //int layer = (int)(floor(log2((float)(treeIdx))));
    //int axis = layer % 3;
    int lesserChildIdx = -1;
    int greaterChildIdx = -1;

    //bitonicSort(startingIdx, endingIdx, axis, particles, totalList); now done beforehand in a seperate kernel call

    int hasGreater = (int)(middleIdx < endingIdx);
    int noGreater = 1 - hasGreater;
    int hasLesser = (int)(middleIdx > startingIdx);
    int noLesser = 1 - hasLesser;

    lesserChildIdx = ((treeIdx << 1) * hasLesser) + (noLesser * -1);
    greaterChildIdx = (((treeIdx << 1) + 1) * hasGreater) + (noGreater * -1);

    output[idx * 2] = ((int4)(lesserChildIdx, startingIdx, middleIdx - 1, 0));
    output[(idx * 2) + 1] = ((int4)(greaterChildIdx, middleIdx + 1, endingIdx, 0));

    for(int i = startingIdx; i <= middleIdx-1; i++) {
        nodeIndexList[i] = lesserChildIdx;
    }
    for(int i = middleIdx+1; i <= endingIdx; i++) {
        nodeIndexList[i] = greaterChildIdx;
    }

    struct KDNode nodeToAdd = {
        indexF4(&particles[totalList[middleIdx]].position, axis),
        totalList[middleIdx], greaterChildIdx - 1, lesserChildIdx - 1};
    theTree[treeIdx - 1] = nodeToAdd;

    
    if(false)printf("\nidx: %i, treeIdx: %i\nstartingIdx %i, middleIdx: %i, endingIdx: %i\nhG %i, nG %i, hL %i, nL %i\ninput: %i, %i, %i, output: %i, %i, %i | %i, %i, %i\n", 
        idx, treeIdx, 
        startingIdx, middleIdx, endingIdx, 
        hasGreater, noGreater, hasLesser, noLesser,
        input[idx].x, input[idx].y, input[idx].z,
        output[idx * 2].x, output[idx * 2].y, output[idx * 2].z,
        output[(idx * 2) + 1].x, output[(idx * 2) + 1].y, output[(idx * 2) + 1].z
    );

}

// slide 25 https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/opencl.2pp.pdf
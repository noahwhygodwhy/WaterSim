#include "Shared.cl"
#include "rand.cl"
//#include "QuickSort.cl"

// TODO: https://www.diva-portal.org/smash/get/diva2:703754/FULLTEXT01.pdf

// page 29

const float NEIGHBOR_RADIUS = 2.0f;
const float NEIGHBOR_RADIUS_SQUARED = 4.0f;

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

float distance_squared(float3 a, float3 b) {
  float3 c = a - b;
  return (c.x * c.x) + (c.y + c.y) + (c.z * c.z);
}

__kernel void computeDP(__global struct Particle *particles,
                        __global struct KDNode *theTree, float deltaTime) {

  int idx = get_global_id(0);

  if (idx != 2) {
    return;
  }
  particles[idx].velocity = (float3)(0.0, 0.0, 1.0);
  uint currIdx = 1;
  float3 theBall = particles[idx].position;

  // you can backtrack cause its stored implicitly, you can math the parent
  // node, and choose to ignore children

  uint lesserTraversals = 0;
  uint greaterTraversals = 0;

  while (currIdx > 0) { // or while(true)???
    struct KDNode currNode = theTree[currIdx - 1];
    int layer = (int)(floor(log2((float)(currIdx))));
    int axis = layer % 3;
    float dsquared =
        distance_squared(theBall, particles[currNode.pointIdx].position);
    if ((dsquared <= NEIGHBOR_RADIUS_SQUARED) && (currNode.pointIdx != idx)) {
      float distance = sqrt(dsquared);
      particles[currIdx - 1].velocity = (float3)(1.0, 1.0, 0.0); // temp
      // DO CALCULATIONS INVOLVING NEIGHBORS HERE
    }

    float val = indexF3(theBall, axis);
    int tooCloseToCall = fabs(val - currNode.value) <= NEIGHBOR_RADIUS;

    int possiblyGreater = tooCloseToCall || (val > currNode.value);
    int possiblyLesser = tooCloseToCall || (val < currNode.value);

    int goingLesser = possiblyLesser && !(1 & lesserTraversals);
    int goingGreater = !goingLesser && possiblyGreater && !(1 & greaterTraversals);
    int goingParent = !goingLesser && !goingGreater;

    lesserTraversals = lesserTraversals << (goingGreater || goingLesser);
    greaterTraversals = greaterTraversals << (goingGreater || goingLesser);
    lesserTraversals = lesserTraversals >> goingParent;
    greaterTraversals = greaterTraversals >> goingParent;

    currIdx = (goingLesser * currNode.lesserChild) +
              (goingGreater * currNode.greaterChild) +
              (goingParent * (currIdx >> 1));

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

void gSwap(__global unsigned int *a, __global unsigned int *b) {
  unsigned int t = *a;
  *a = *b;
  *b = t;
}

// https://en.wikipedia.org/wiki/Bitonic_sorter
void bitonicSort(int start, int end, int axis,
                 __global struct Particle *particles,
                 __global unsigned int *totalList) {
  // bool dbg = (get_global_id(0)==0);

  // if(dbg)printf("start: %i, end: %i, n: %i\n", start, end, (start-end)+1);

  int n = (end - start) + 1;

  for (int k = 2; k <= n; k *= 2) {      // k is doubled every iteration
    for (int j = k / 2; j > 0; j /= 2) { // j is halved at every iteration, with
                                         // truncation of fractional parts
      for (int i = 0; i < n; i++) {
        int l = i ^ j; // in C-like languages this is "i ^ j"

        // printf("idx: %i, l: %i, i: %i, k: %i", get_global_id(0), l, i, k);

        if (l > i) {
          if (!(((i & k) == 0) ^
                (indexF3(particles[totalList[i]].position, axis) >
                 indexF3(particles[totalList[l]].position, axis)))) {
            gSwap(totalList + i, totalList + l);
          }
        }
      }
    }
  }
}

__kernel void makeKDTree(__global struct Particle *particles,
                         __global unsigned int *totalList,
                         __global struct KDNode *theTree, __global int3 *input,
                         __global int3 *output, int inputCount) {
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

  bitonicSort(startingIdx, endingIdx, axis, particles, totalList);

  int hasGreater = (int)(middleIdx < endingIdx);
  int noGreater = 1 - hasGreater;
  int hasLesser = (int)(middleIdx > startingIdx);
  int noLesser = 1 - hasLesser;

  lesserChildIdx = ((treeIdx << 1) * hasLesser) + (noLesser * -1);
  output[idx * 2] = ((int3)(lesserChildIdx, startingIdx, middleIdx - 1));

  output[(idx * 2) + 1] = ((int3)(greaterChildIdx, middleIdx + 1, endingIdx));

  lesserChildIdx = (((treeIdx << 1) + 1) * hasGreater) + (noGreater * -1);

  struct KDNode nodeToAdd = {
      indexF3(particles[totalList[middleIdx]].position, axis),
      totalList[middleIdx], greaterChildIdx - 1, lesserChildIdx - 1};
  theTree[treeIdx - 1] = nodeToAdd;
}

// slide 25 https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/opencl.2pp.pdf
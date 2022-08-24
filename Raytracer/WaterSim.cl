#include "Shared.cl"
#include "rand.cl"
//#include "QuickSort.cl"

// TODO: https://www.diva-portal.org/smash/get/diva2:703754/FULLTEXT01.pdf

// page 29

// //TODO: tweak these
// __constant float NEIGHBOR_RADIUS = 0.5f;
// __constant float REST_DENSITY = 1.0f;
// __constant float GAS_STIFFNESS = 1.0f;
// __constant float3 GRAVITY = (float3)(0, -0.1f, 0);
// __constant float PARTICLE_MASS = 1.0f;

//tweaked
__constant float NEIGHBOR_RADIUS = 1.0f;
__constant float REST_DENSITY = 1.0f;
__constant float GAS_STIFFNESS = 1.0f;
__constant float3 GRAVITY = (float3)(0, 0.0f, 0);
__constant float PARTICLE_MASS = 1.0f;

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

float vector_length_2(float3 c) {
    return (c.x * c.x) + (c.y * c.y) + (c.z * c.z);
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
//although to be fair, he claimed ignorance to how it works as well
//just that it he was able to get it working
__kernel void bitonicSort(
    __global struct Particle *particles,
    __global unsigned int *totalList,
    __global unsigned int *indexOfNodeList,
    int axis,
    int blockWidth,
    int reflect,
    int numParticles
){

    uint mask = ((uint)(1) << blockWidth) - 1;
    uint offset = (uint)(1) << blockWidth;
    uint limit = nextPowerOfTwo(numParticles) / 2;
    uint init = get_local_id(0)+(get_local_size(0)*get_group_id(0));
    uint iter = get_local_size(0) * get_num_groups(0);
    for (
            uint i = init;
            i < limit;
            i += iter
    ) {
        uint j = ((i & ~mask) << 1) + (i & mask);
        uint k = ((i & ~mask) << 1) + ((reflect ? ~i : i) & mask) + offset;

        if (indexOfNodeList[j] == indexOfNodeList[k] && k < numParticles) {
            
            float jVal = indexF4(&particles[totalList[j]].position, axis);
            float kVal = indexF4(&particles[totalList[k]].position, axis);
            if (jVal > kVal) {
                
                int swapSpace = totalList[j];
                totalList[j]= totalList[k];
                totalList[k] = swapSpace;
            }
        }
    }            
}


__constant float POLY6_LEFT_SIDE = 1.56668147106f;
__constant float SPIKY_LEFT_SIDE = -14.3239449f;
__constant float VISCOSITY_LEFT_SIDE = 14.3239449f;
float Wpoly6(float r2, float h2, float h9) {
    float inter = pow(h2-r2, 3);
    return (POLY6_LEFT_SIDE * inter)/h9;
}
float3 deltaWspiky(float3 theVec, float r, float h, float h6){
    float3 inter = (theVec/r) * ((h-r)*(h-r));
    return (VISCOSITY_LEFT_SIDE*inter)/h6;
}

float deltaTwoWviscosity(float r, float h, float h6) {
    float inter = (h-r);
    return (VISCOSITY_LEFT_SIDE * inter)/h6;
}




__kernel void computeDP(
    __global struct Particle *particles,
    __global struct KDNode *theTree,
    int frameNumber) {

    int idx = get_global_id(0);

    uint currIdx = 1;
    float3 theBall = particles[idx].position.xyz;

    uint lesserTraversals = 0;
    uint greaterTraversals = 0;
    int traveledNodes = 0;
    float h = NEIGHBOR_RADIUS;
    float h9 = pow(NEIGHBOR_RADIUS, 9);
    float h2 = pow(NEIGHBOR_RADIUS, 2);

    bool lastParent = false;
    float newDensity = 0;
    while (currIdx > 0) { 
        struct KDNode currNode = theTree[currIdx - 1];
        int layer = (int)(floor(log2((float)(currIdx))));
        int axis = layer % 3;
        
        float3 theVec = particles[currNode.pointIdx].position.xyz-theBall;
        float dsquared = vector_length_2(theVec);
        if ((dsquared <= h2) && (currNode.pointIdx != idx) && !lastParent) {
            float distance = sqrt(dsquared);
            newDensity += PARTICLE_MASS* Wpoly6(distance, h2, h9);
            
            //particles[currNode.pointIdx].velocity = (float4)(1.0, 1.0, 0.0, 0.0);// * (distance/NEIGHBOR_RADIUS); // temp
        }

        float val = indexF3(theBall, axis);
        int tooCloseToCall = fabs(val - currNode.value) <= NEIGHBOR_RADIUS;

        int possiblyGreater = (tooCloseToCall || (val > currNode.value)) && currNode.greaterChild >= 0;
        int possiblyLesser = (tooCloseToCall || (val < currNode.value)) && currNode.lesserChild >= 0;

        lastParent = false;
        if(possiblyLesser && !(1&lesserTraversals)) {
                lesserTraversals |= 1;
                lesserTraversals = lesserTraversals<<1;
                greaterTraversals = greaterTraversals<<1;
                currIdx = currNode.lesserChild+1;
            }
        else {
            if (possiblyGreater && !(1 & greaterTraversals)) {
                greaterTraversals |= 1;
                lesserTraversals = lesserTraversals << 1;
                greaterTraversals = greaterTraversals << 1;
                currIdx = currNode.greaterChild + 1;
            }
            else {
                lastParent = true;
                currIdx = currIdx / 2;
                lesserTraversals = lesserTraversals >> 1;
                greaterTraversals = greaterTraversals >> 1;
            }
        }
    }
    particles[idx].dp.x = newDensity;
    //printf("idx: %i,density %f\n", idx, newDensity);
    particles[idx].dp.y = GAS_STIFFNESS * (newDensity-REST_DENSITY);
}

__kernel void computeForce(
    __global struct Particle *particles,
    __global struct KDNode *theTree) {

    int idx = get_global_id(0);

    uint currIdx = 1;
    float3 theBall = particles[idx].position.xyz;

    uint lesserTraversals = 0;
    uint greaterTraversals = 0;
    int traveledNodes = 0;
    float h = NEIGHBOR_RADIUS;
    //float h9 = pow(NEIGHBOR_RADIUS, 9);
    float h2 = pow(h, 2);
    float h6 = pow(h, 6);
    bool lastParent = false;
    
    float3 fPressure = (float3)(0.0f, 0.0f, 0.0f);
    float3 fViscosity = (float3)(0.0f, 0.0f, 0.0f);

    while (currIdx > 0) { 
        struct KDNode currNode = theTree[currIdx - 1];
        int layer = (int)(floor(log2((float)(currIdx))));
        int axis = layer % 3;
        float3 theVec = particles[currNode.pointIdx].position.xyz-theBall;
        float dsquared = vector_length_2(theVec);
        if ((dsquared <= h2) && (currNode.pointIdx != idx) && !lastParent) {
            
            float distance = sqrt(dsquared);
            float pLeftSide = PARTICLE_MASS * ((particles[idx].dp.y + particles[currNode.pointIdx].dp.y)/(2*particles[currNode.pointIdx].dp.y));
            float3 vLeftSide = PARTICLE_MASS * ((particles[idx].velocity - particles[currNode.pointIdx].velocity).xyz/(particles[currNode.pointIdx].dp.y));
            
            float3 fPressure = fPressure - pLeftSide*deltaWspiky(theVec, distance, h, h6);
            float3 fViscosity = fViscosity + vLeftSide* deltaTwoWviscosity(distance, h, h6);

            //float deltaTwoWviscosity(float4 theVec, float r, float h, float h6) {
            //TODO: the force things on page 35 of https://www.diva-portal.org/smash/get/diva2:703754/FULLTEXT01.pdf
        }

        float val = indexF3(theBall, axis);
        int tooCloseToCall = fabs(val - currNode.value) <= NEIGHBOR_RADIUS;

        int possiblyGreater = (tooCloseToCall || (val > currNode.value)) && currNode.greaterChild >= 0;
        int possiblyLesser = (tooCloseToCall || (val < currNode.value)) && currNode.lesserChild >= 0;

        lastParent = false;
        if(possiblyLesser && !(1&lesserTraversals)) {
                lesserTraversals |= 1;
                lesserTraversals = lesserTraversals<<1;
                greaterTraversals = greaterTraversals<<1;
                currIdx = currNode.lesserChild+1;
            }
        else {
            if (possiblyGreater && !(1 & greaterTraversals)) {
                greaterTraversals |= 1;
                lesserTraversals = lesserTraversals << 1;
                greaterTraversals = greaterTraversals << 1;
                currIdx = currNode.greaterChild + 1;
            }
            else {
                lastParent = true;
                currIdx = currIdx / 2;
                lesserTraversals = lesserTraversals >> 1;
                greaterTraversals = greaterTraversals >> 1;
            }
        }
    }
    //f=m*a
    float3 fGravity = GRAVITY * PARTICLE_MASS;
    //particles[idx].force = (float4)(0 + 0 + fGravity, 0);
    //printf("updated force of %i: %f, %f, %f\n",idx, fGravity.x, fGravity.y, fGravity.z);
    particles[idx].force = (float4)(fPressure + fViscosity + fGravity, 0);

}

__kernel void updateVX(
    __global struct Particle *particles,
    float deltaTime
){
    int idx = get_global_id(0);
    //if(idx == 0){printf("delta time %f\n", deltaTime);}

    
    particles[idx].velocity += particles[idx].force/PARTICLE_MASS;//particles[idx].pressure;
    particles[idx].position = particles[idx].position + (particles[idx].velocity * deltaTime);
    //  printf("particle %i at new position %f, %f, %f\n", 
    // idx,
    // particles[idx].position.x, particles[idx].position.y, particles[idx].position.z
    // );
    // particles[idx].position.x = 10;
    // particles[idx].position.y = 10;
    // particles[idx].position.z = 10;
    // particles[idx].position.w = 10;
    // particles[idx].velocity.x = 10;
    // particles[idx].velocity.y = 10;
    // particles[idx].velocity.z = 10;
    // particles[idx].velocity.w = 10;
    // particles[idx].force.x = 10;
    // particles[idx].force.y = 10;
    // particles[idx].force.z = 10;
    // particles[idx].force.w = 10;
    // particles[idx].density = 10;
    //particles[idx].pressure = 10;
    //particles[idx].fillerOne = 10;
    //particles[idx].fillerTwo = 10;
    // printf("particle %i moving at %f, %f, %f, at new position %f, %f, %f due to pressure %f and force %f, %f %f\n", 
    // idx,
    // particles[idx].velocity.x, particles[idx].velocity.y, particles[idx].velocity.z,
    // particles[idx].position.x, particles[idx].position.y, particles[idx].position.z,
    // particles[idx].pressure,
    // particles[idx].force.x, particles[idx].force.y, particles[idx].force.z
    // );
    // particles[idx].position = min(particles[idx].position, (float4)(9, 9, 9, 0));
    // particles[idx].position = max(particles[idx].position, (float4)(1, 1, 1, 0));

}

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
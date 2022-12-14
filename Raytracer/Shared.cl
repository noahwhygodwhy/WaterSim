#ifndef SHARED_H
#define SHARED_H









#ifdef CPP
    #include <cstdint>
    #include <glm/glm.hpp>
    using namespace glm;
    typedef vec4 float4;
    typedef vec3 float3;

    #define STRUCT_HEADER typedef struct alignas(16)
#else
    #define STRUCT_HEADER typedef struct 
#endif


STRUCT_HEADER OtherData {
    float3 boxSize;
    float deltaTime;
    float3 gravity;
} OtherData;

STRUCT_HEADER KDNode {
	float value;
	unsigned int pointIdx; //index into balls
	int greaterChild; //index into the tree
	int lesserChild;//index into the tree
} KDNode;

STRUCT_HEADER Particle {
    float4 position;
    float4 velocity;
    float4 force;
    float4 dp; //density is the first, pressure is the second, and the other two are just filler
} Particle;



#endif
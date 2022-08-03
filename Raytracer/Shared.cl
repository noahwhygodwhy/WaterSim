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



#endif
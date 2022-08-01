#include "rand.cl"

__kernel void watersim(
    __global float3* positions,
    __global float3* velocities
    )  {
    int idx = get_global_id(0);    
}


//slide 25 https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/opencl.2pp.pdf
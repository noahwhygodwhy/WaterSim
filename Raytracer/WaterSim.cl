#include "rand.cl"

__kernel void watersim(
    __global unsigned int* sourceBath,
    __global unsigned int* destBath
    )  {

    

    int bathX = get_global_size(0);
    int bathY = get_global_size(1);

    int pixelX = get_global_id(0);
    int pixelY = get_global_id(1);

    bool dbg = (pixelX == 0)&&(pixelY==0);

    int pixelIdx = pixelX+(bathX*pixelY);
    
    int newPixelX = pixelX + 1;
    if(newPixelX >= bathX) {
        newPixelX = 0;
    }
    int newPixelIdx = newPixelX+(bathX*pixelY);

    //if(dbg)printf("%i, %i\n", pixelIdx, newPixelIdx);
    //if(dbg)printf("source: %i\n", sourceBath[pixelIdx]);
        
    destBath[newPixelIdx] = sourceBath[pixelIdx];
    
}


__kernel void render(
    __global unsigned int* sourceBath,
    write_only image2d_t frameTexture
) {



    const float4 colors[]  = {
        (float4)(1.0, 1.0, 1.0, 1.0),
        (float4)(0.0, 0.0, 1.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
        (float4)(1.0, 1.0, 0.0, 1.0),
    };
    int bathX = get_global_size(0);
    int bathY = get_global_size(1);

    int pixelX = get_global_id(0);
    int pixelY = get_global_id(1);

    int pixelIdx = pixelX+(bathX*pixelY);

    write_imagef(frameTexture, (int2) (pixelX, pixelY),  colors[sourceBath[pixelIdx]]);


}




//slide 25 https://web.engr.oregonstate.edu/~mjb/cs575/Handouts/opencl.2pp.pdf
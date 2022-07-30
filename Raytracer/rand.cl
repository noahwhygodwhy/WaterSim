
float randwithfloat2(float2 co){

    float2 t = {12.9898, 78.233};
    float a = dot(co, t);
    float b = sin(a);
    float c;
    return modf(b * 43758.5453, &c);
}


uint MWC64X(ulong *state)
{
    uint c=(*state)>>32, x=(*state)&0xFFFFFFFF;
    *state = x*((ulong)4294883355U) + c;
    return x^c;
}


float rand(ulong *state){
    
    float x = MWC64X(state)/((float)(UINT_MAX));
    return x;
}

float randDubThree(ulong* state) {
    return (rand(state)*2.0f)-1.0f;

}
#version 460

//layout (location = 0) in vec2 aPos;

vec2 coords[] = {vec2(0.0f, 0.0f), vec2(2.0f, 0.0f), vec2(0.0f, 2.0f)};

out vec2 fragUV;

void main()
{
    fragUV = coords[gl_VertexID];
    gl_Position = vec4((coords[gl_VertexID].x*2.0f)-1.0f,(coords[gl_VertexID].y*2.0f)-1.0f, 0.0f, 1.0);
}
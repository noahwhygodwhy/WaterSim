#version 460

layout (location = 0) in vec4 aPos;

uniform mat4 projection;
uniform mat4 view;

out float fragInd;

void main()
{
    fragInd = aPos.w;
    gl_Position = projection * view * vec4(aPos.xyz, 1.0);
}
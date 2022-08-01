#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragNormal;
out vec3 fragPos;  


void main()
{
    fragNormal = aNorm;
    fragPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
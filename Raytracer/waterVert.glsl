#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVel;
layout (location = 2) in vec3 aDens;
layout (location = 3) in vec3 aPres;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragVel;

void main()
{
    fragVel = aDens;
    fragVel = aPres;
    fragVel = aVel;
    gl_Position = projection * view * vec4(aPos.xyz, 1.0);
}
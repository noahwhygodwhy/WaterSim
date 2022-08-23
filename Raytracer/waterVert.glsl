#version 460

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aVel;
layout (location = 2) in float aDens;
layout (location = 3) in float aPres;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    //fragVel = aDens;
    //fragVel = aPres;
    //fragVel = aVel.xyz;
    gl_Position = projection * view * vec4(aPos.xyz, 1.0);
}
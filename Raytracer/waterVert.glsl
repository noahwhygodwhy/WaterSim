#version 460




layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aVel;
layout (location = 2) in vec4 aForce;
layout (location = 3) in vec4 aDP;

uniform mat4 projection;
uniform mat4 view;

void main()
{

    gl_Position = projection * view * vec4(aPos.xyz, 1.0);
}
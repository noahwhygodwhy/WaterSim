#version 460

in vec2 fragUV;

uniform sampler2D textur;

out vec4 FragColor;

void main() 
{
    FragColor = texture2D(textur, fragUV);
}
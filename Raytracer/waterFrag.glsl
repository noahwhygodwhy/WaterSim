#version 460

out vec4 FragColor;

in float fragInd;

void main() 
{
    FragColor = vec4(fragInd, 0.0, 1.0, 1.0);
    if(fragInd > 0.6 && fragInd < 0.8) {   
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
}
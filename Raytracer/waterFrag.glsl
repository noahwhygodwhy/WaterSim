#version 460

out vec4 FragColor;


void main() 
{
    FragColor = vec4(0.4, 0.4, 0.9, 1.0);
//
//    FragColor = vec4(fragInd, 0.0, 1.0, 1.0);
//    if(fragInd > 0.6 && fragInd < 0.8) {   
//        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
//    }
//    if(fragInd > 0.3 && fragInd < 0.5) {   
//        FragColor = vec4(1.0, 1.0, 0.0, 1.0);
//    }
}
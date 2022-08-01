#version 460

in vec3 fragNormal;
in vec3 fragPos;  

out vec4 FragColor;

void main() 
{

    vec3 lightPos = vec3(2.5, 2.5, 2.5);
    vec3 objectColor = vec3(1.0, 1.0, 1.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;


    
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;

    FragColor = vec4(result, 1.0);
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);

}
#version 330   

in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    

out vec4 FragColor;       

uniform vec3 lightColor;
uniform vec3 lightDirection;

void main()
{
    float diffCoefficient = max(dot(Normal0, normalize(lightDirection)), 0.4);
    vec3 diffuse = diffCoefficient * lightColor;
    
    FragColor = vec4(diffuse, 1.0);
}
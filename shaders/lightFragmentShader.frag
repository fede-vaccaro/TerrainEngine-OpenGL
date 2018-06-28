#version 330 core

uniform vec3 u_LightColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(u_LightColor, 1.0f);
};
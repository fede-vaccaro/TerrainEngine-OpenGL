#version 330 core

uniform vec3 u_LightColor;

out vec4 FragColor;

void main()
{

	FragColor = vec4(u_LightColor, 1.0f);
	if( u_LightColor == vec3(0.0)){
		FragColor = vec4(1.0);	
	}
};
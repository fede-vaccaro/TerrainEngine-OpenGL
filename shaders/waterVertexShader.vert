#version 330 core                                                                               

layout (location = 0) in vec3 aPos;                                           
layout (location = 1) in vec3 aNor;   
layout (location = 2) in vec2 aTex;

uniform mat4 modelMatrix;
uniform mat4 gVP;

out vec3 Normal;
out vec4 clipSpaceCoords;
out vec2 TexCoords;
out vec4 position;

void main(){
	TexCoords = aTex;
	Normal = aNor;
	position = modelMatrix*vec4(aPos, 1.0);
	clipSpaceCoords = gVP*position;
	gl_Position = clipSpaceCoords;
}
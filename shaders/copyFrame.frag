#version 330 core

layout (location = 0) out vec4 alphaness;
layout (location = 1) out vec4 color;

in vec2 TexCoords;

uniform sampler2D colorTex;
uniform sampler2D alphanessTex;


void main()
{
	color = texture(colorTex, TexCoords);
	alphaness = texture(alphanessTex, TexCoords);

	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}  
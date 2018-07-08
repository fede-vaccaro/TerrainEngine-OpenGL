#version 330 core
layout (location = 0) in vec2 aPos;

out vec2 Coords;

void main()
{
	//gl_PointSize = 1.0;
	Coords = aPos;
    gl_Position = vec4(aPos,0.0,1.0);
}
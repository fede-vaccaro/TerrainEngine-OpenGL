#version 330 core
layout (location = 0) in vec2 aPos;

out vec2 WorldPos_CS_in;                                                                      


void main()
{
	//gl_PointSize = 1.0;
	WorldPos_CS_in = aPos;
    //gl_Position = vec4(WorldPos_CS_in, 0.0, 1.0);
}
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 transformation;
uniform mat4 scale;

void main()
{
  //gl_Position = transformation * vec4(aPos, 0.0 ,1.0);
  gl_Position = transformation * scale * vec4(aPos, 0.0 ,1.0);
  //gl_Position = vec4(aPos, 0.0 ,1.0);
  gl_Position.z = 0.0;
}

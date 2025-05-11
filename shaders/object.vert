#version 330   

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 uMVP;
uniform mat4 uWorld;

out vec2 TexCoord0;
out vec3 Normal0;

void main()
{
    gl_Position = uMVP * vec4(aPosition, 1.0);
    TexCoord0 = aTexCoord;
    Normal0 = aNormal;
}

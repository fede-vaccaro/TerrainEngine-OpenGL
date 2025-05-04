#version 330

in vec3 v_col;
out vec4 frag_color;

void main()
{
    frag_color = vec4(v_col.xyz, 1.0);
}
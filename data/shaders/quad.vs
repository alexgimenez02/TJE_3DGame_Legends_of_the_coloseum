#version 330 core

in vec3 a_vertex;
in vec2 a_coord;
out vec2 v_uv;

void main()
{
    v_uv = a_coord;
    gl_Position = vec4(a_vertex, 1.0);
}
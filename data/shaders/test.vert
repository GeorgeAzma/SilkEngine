#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out VertexOutput 
{
    vec3 color;
} vertex_output;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    vertex_output.color = in_color;
}
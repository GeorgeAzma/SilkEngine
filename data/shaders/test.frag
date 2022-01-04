#version 450

layout(location = 0) in VERTEX_OUTPUT 
{
    vec3 color;
} fragment_input;


layout(location = 0) out vec4 color;

void main()
{
    color = vec4(fragment_input.color, 1.0);
}
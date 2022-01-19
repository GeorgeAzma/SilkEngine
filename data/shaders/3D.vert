#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinates;
layout(location = 2) in vec3 in_normal;

layout(location = 3) in mat4 in_transform;

layout(binding = 0) uniform Transforms
{
    mat4 projection_view;
} transforms;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinates;
} vertex_output;

void main()
{
    gl_Position = transforms.projection_view * in_transform * vec4(in_position, 1.0);
    vertex_output.texture_coordinates = in_texture_coordinates;
}
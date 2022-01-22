#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinates;
layout(location = 2) in vec3 in_normal;

//Instanced
layout(location = 3) in mat4 in_transform;
layout(location = 7) in uint texture_index;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinates;
} vertex_output;

layout(binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    gl_Position = global_uniform.projection_view * in_transform * vec4(in_position, 1.0);
    vertex_output.texture_coordinates = in_texture_coordinates;
}
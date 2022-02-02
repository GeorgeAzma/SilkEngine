#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinate;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in uint in_texture_index;
layout(location = 4) in vec4 in_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
    flat uint texture_index;
    vec4 color;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    vec3 camera_position;
    float time;
    vec3 camera_direction;
    float delta_time;
    uvec2 resolution;
    uint frame;
    uint flags;
} global_uniform;

void main()
{
    gl_Position = global_uniform.projection_view * vec4(in_position, 1.0);
    vertex_output.texture_coordinate = in_texture_coordinate;
    vertex_output.texture_index = in_texture_index;
    vertex_output.color = in_color;
}
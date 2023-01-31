layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 vertex_texture_coordinate;
layout(location = 2) in vec4 vertex_color;

//Instanced
layout(location = 3) in mat4 instance_transform;
layout(location = 7) in uint instance_image_index;
layout(location = 8) in vec4 instance_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
    flat uint instance_image_index;
    flat vec4 instance_color;
    vec4 color;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    vertex_output.texture_coordinate = vertex_texture_coordinate;
    vertex_output.instance_image_index = instance_image_index;
    vertex_output.instance_color = instance_color;
    vertex_output.color = vertex_color;
    gl_Position = global_uniform.projection_view * instance_transform * vec4(vertex_position, 0.0, 1.0);
}
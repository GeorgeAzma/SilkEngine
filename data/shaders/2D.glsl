#type vertex
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinate;
layout(location = 2) in vec3 in_normal;

//Instanced
layout(location = 3) in mat4 in_transform;
layout(location = 7) in uint in_texture_index;
layout(location = 8) in vec4 in_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
    uint texture_index;
    vec4 color;
} vertex_output;

void main()
{
    vertex_output.texture_coordinate = in_texture_coordinate;
    vertex_output.texture_index = in_texture_index;
    vertex_output.color = in_color;

    gl_Position = global_uniform.projection_view * in_transform * vec4(in_position, 1.0);
}

#type fragment

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat uint texture_index;
    flat vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{
    color = texture(images[fragment_input.texture_index], fragment_input.texture_coordinate) * fragment_input.color;
}
#type vertex
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texture_coordinate;
layout(location = 2) in vec4 in_vcolor;

//Instanced
layout(location = 3) in mat4 in_transform;
layout(location = 7) in uint in_image_index;
layout(location = 8) in vec4 in_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
    flat uint image_index;
    flat vec4 color;
    vec4 vcolor;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    mat4 projection_view2D;
} global_uniform;

void main()
{
    vertex_output.texture_coordinate = in_texture_coordinate;
    vertex_output.image_index = in_image_index;
    vertex_output.color = in_color;
    vertex_output.vcolor = in_vcolor;
    gl_Position = global_uniform.projection_view2D * in_transform * vec4(in_position, 0.0, 1.0);
}

#type fragment

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat uint image_index;
    flat vec4 color;
    vec4 vcolor;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{
    color = texture(images[fragment_input.image_index], fragment_input.texture_coordinate) * fragment_input.color * fragment_input.vcolor;
    if(color.a <= 0.01)
        discard;
}
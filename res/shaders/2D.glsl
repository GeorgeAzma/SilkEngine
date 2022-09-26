#depth_compare_op <=

#type vertex
layout(location = 0) in vec2 V_position;
layout(location = 1) in vec2 V_texture_coordinate;
layout(location = 2) in vec4 V_color;

//Instanced
layout(location = 3) in mat4 I_transform;
layout(location = 7) in uint I_image_index;
layout(location = 8) in vec4 I_color;

layout(location = 0) out VertexOutput 
{
    vec2 V_texture_coordinate;
    flat uint I_image_index;
    flat vec4 I_color;
    vec4 V_color;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    mat4 projection_view2D;
} global_uniform;

void main()
{
    vertex_output.V_texture_coordinate = V_texture_coordinate;
    vertex_output.I_image_index = I_image_index;
    vertex_output.I_color = I_color;
    vertex_output.V_color = V_color;
    gl_Position = global_uniform.projection_view2D * I_transform * vec4(V_position, 0.0, 1.0);
}

#type fragment
layout(location = 0) in VertexOutput 
{
    vec2 V_texture_coordinate;
    flat uint I_image_index;
    flat vec4 I_color;
    vec4 V_color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{
    color = fragment_input.I_color * fragment_input.V_color;
    if (fragment_input.I_image_index != 0)
        color *= texture(images[fragment_input.I_image_index], fragment_input.V_texture_coordinate);
    if (color.a <= 0.01)
        discard;
}
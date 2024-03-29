layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 vertex_uv;
layout(location = 2) in vec4 vertex_color;

layout(location = 3) in mat4 instance_transform;
layout(location = 7) in vec4 instance_color;
layout(location = 8) in int instance_image_index;

layout(location = 0) out VertexOutput 
{
    vec2 uv;
    vec4 color;
    flat int instance_image_index;
} vert_out;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    mat4 projection_view2D;
} global_uniform;

void main()
{
    vert_out.uv = vertex_uv;
    vert_out.color = vertex_color * instance_color;
    vert_out.instance_image_index = instance_image_index;
    gl_Position = global_uniform.projection_view2D * instance_transform * vec4(vertex_position, 0.0, 1.0);
}
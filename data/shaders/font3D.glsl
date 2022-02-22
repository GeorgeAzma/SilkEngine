#type vertex
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinate;

//Instanced
layout(location = 2) in mat4 in_transform;
layout(location = 6) in vec4 in_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
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
    vertex_output.texture_coordinate = in_texture_coordinate;
    vertex_output.color = in_color;
    gl_Position = global_uniform.projection_view * in_transform * vec4(in_position, 1.0);
}

#type fragment

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D atlas;

void main()
{
   color = texture(atlas, fragment_input.texture_coordinate) * fragment_input.color;
}
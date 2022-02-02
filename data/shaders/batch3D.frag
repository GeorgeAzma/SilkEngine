#version 450

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat uint texture_index;
    vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler[32];

void main()
{
    color = texture(texture_sampler[fragment_input.texture_index], fragment_input.texture_coordinate);
    color *= fragment_input.color;
    if(color.a < 1 / 255)
        discard;
}
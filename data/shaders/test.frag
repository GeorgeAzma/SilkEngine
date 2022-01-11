#version 450

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinates;
    vec3 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(binding = 1) uniform sampler2D texture_sampler;

void main()
{
    color = vec4(fragment_input.color, 1.0);
    color *= texture(texture_sampler, fragment_input.texture_coordinates);
}
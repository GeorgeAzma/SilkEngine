#version 450

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinates;
} fragment_input;

layout(location = 0) out vec4 color;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(std140, binding = 2) buffer MyBuffer
{
    vec3 color;
} buff;

void main()
{
    color = texture(texture_sampler, fragment_input.texture_coordinates) * vec4(buff.color, 1);
}
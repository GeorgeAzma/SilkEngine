#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinates;
    flat uint texture_index;
    flat vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler[32];

void main()
{
    color = texture(texture_sampler[fragment_input.texture_index], fragment_input.texture_coordinates);
    color *= fragment_input.color;
    if(color.a < 1 / 255)
        discard;
}
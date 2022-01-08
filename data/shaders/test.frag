#version 450

layout(location = 0) in VertexOutput 
{
    vec3 color;
} fragment_input;

layout(binding = 0) uniform TransformMatrices
{
    vec3 color;
} transform_matrices;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(fragment_input.color, 1.0);
}
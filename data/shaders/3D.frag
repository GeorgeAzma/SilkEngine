#version 450

#define AMBIENT 0.1

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    vec3 normal;
    flat uint texture_index;
    flat vec4 color;
    vec3 world_position;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler[32];

void main()
{
    color = texture(texture_sampler[fragment_input.texture_index], fragment_input.texture_coordinate);
    color *= fragment_input.color;
    if(color.a < 1 / 255)
        discard;
        
    color.rgb = vec3(fragment_input.texture_coordinate, 0);return;
    //TODO: PBR
    if(fragment_input.normal == vec3(0))
        return;

    float lighting = 0.0;

    vec3 dir = normalize(fragment_input.world_position - vec3(1000, 2000, 500));
    lighting += max(dot(fragment_input.normal, dir), 0);

    color.rgb *= lighting;
}
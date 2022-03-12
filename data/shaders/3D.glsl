#type vertex
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coordinate;
layout(location = 2) in vec3 in_normal;

//Instanced
layout(location = 3) in mat4 in_transform;
layout(location = 7) in uint in_texture_index;
layout(location = 8) in vec4 in_color;

layout(location = 0) out VertexOutput 
{
    vec2 texture_coordinate;
    vec3 normal;
    flat uint texture_index;
    flat vec4 color;
    vec3 world_position;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    vertex_output.texture_coordinate = in_texture_coordinate;
    vertex_output.normal = in_normal;
    vertex_output.texture_index = in_texture_index;
    vertex_output.color = in_color;
    const vec4 world_position = in_transform * vec4(in_position, 1.0);
    vertex_output.world_position = world_position.xyz;

    gl_Position = global_uniform.projection_view * world_position;
}

#type fragment
//TODO: Remove hardcoded materials
#define AMBIENT 0.01
#define METALLIC 0.0
#define ROUGHNESS 0.3

layout (constant_id = 0) const bool lit = true;

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    vec3 normal;
    flat uint texture_index;
    flat vec4 color;
    vec3 world_position;
} fragment_input;

#include "light.glsl"

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    mat4 projection_view2D;
    vec3 camera_position;
    float time;
    vec3 camera_direction;
    float delta_time;
    uvec2 resolution;
    uint frame;
    Light lights[MAX_LIGHTS];
} global_uniform;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

//Good HDR filter
vec3 aces(vec3 x)
{
  const float a=2.51;
  const float b=.03;
  const float c=2.43;
  const float d=.59;
  const float e=.14;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.,1.);
}

void main()
{
    if(lit)
    {
        vec4 albedo = texture(images[fragment_input.texture_index + DIFFUSE_TEXTURE], fragment_input.texture_coordinate) * fragment_input.color;
        color.a = albedo.a;
        if(color.a <= 0.01)
            discard;
        
        //TODO: textures mapping (normal, roughness, metallic)
        vec3 normal = fragment_input.normal;
        
        if (normal == vec3(0) || albedo.rgb == vec3(0))
        {
            color.rgb = albedo.rgb;
            return;
        }
        
        normal = normalize(normal);
        
        float metallic = METALLIC;
        
        vec3 to_camera = normalize(global_uniform.camera_position - fragment_input.world_position.xyz); //V
        vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
        
        vec3 ambient = vec3(AMBIENT) * albedo.rgb; // * ao
        vec3 total_light = vec3(0);
        
        for(uint i = 0; i < global_uniform.light_count; ++i)
        {
            Light light = global_uniform.lights[i];
            if (light.color == vec3(0))
                continue;
        
            total_light += PBR(light, normal, to_camera, albedo.rgb, metallic, ROUGHNESS, F0, fragment_input.world_position.xyz);
        }
        color.rgb = aces(total_light + ambient.rgb);
    }
    else
    {
        color = texture(images[fragment_input.texture_index + DIFFUSE_TEXTURE], fragment_input.texture_coordinate) * fragment_input.color;
        if(color.a <= 0.01)
            discard;
    }
}
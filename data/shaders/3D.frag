#version 450

//TODO: Remove hardcoded materials and copy pbr in batch3D
#define AMBIENT 0.01
#define METALLIC 0.0
#define ROUGHNESS 0.3
#define MAX_LIGHTS 64
#define PI 3.14159265359

layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    vec3 normal;
    flat uint texture_index;
    flat vec4 color;
    vec3 world_position;
} fragment_input;

struct Light
{
    vec3 position;
    float linear;
    vec3 direction;
    float quadratic;
    vec3 color;
    float padding;
};

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    vec3 camera_position;
    float time;
    vec3 camera_direction;
    float delta_time;
    uvec2 resolution;
    uint frame;
    uint light_count;
    Light lights[MAX_LIGHTS];
} global_uniform;

layout(set = 1, binding = 0) uniform Material
{
    float ambient;
    float ambient_occlusion;
    float roughness;
    float metallic;
} material;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler[32];

vec3 fresnelSchlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a2 = roughness * roughness * roughness * roughness;
    float NdotH = max(dot(normal, halfway), 0.0);
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float geometrySmith(vec3 normal, vec3 to_camera, vec3 to_light, float roughness)
{
    return geometrySchlickGGX(max(dot(normal, to_camera), 0.0), roughness) 
         * geometrySchlickGGX(max(dot(normal, to_light), 0.0), roughness);
}

vec3 PBR(Light light, vec3 normal, vec3 to_camera, vec3 albedo, float metallic, float roughness, vec3 F0)
{
   vec3 lighting = vec3(0);

   vec3 to_light = normalize(light.position - fragment_input.world_position.xyz); //wi
   vec3 halfway = normalize(to_camera + to_light);
   float attenuation = 1.0;
   if(light.linear != 0 || light.quadratic != 0)
   {
       float distance_to_light = distance(light.position, fragment_input.world_position.xyz);
       attenuation = 1.0 / (1.0 + light.linear * distance_to_light + light.quadratic * (distance_to_light * distance_to_light));
   }

   vec3 radiance = light.color * attenuation;
   float NDF = distributionGGX(normal, halfway, roughness);       
   float geometry_smith = geometrySmith(normal, to_camera, to_light, roughness);   
   vec3 F = fresnelSchlick(max(dot(halfway, to_camera), 0), F0);

   vec3 specular = (NDF * geometry_smith * F) / (4.0 * max(dot(normal, to_camera), 0.0) * max(dot(normal, to_light), 0.0) + 0.0001);  
   vec3 kD = (vec3(1) - F) * (1 - metallic);

   lighting = (kD * albedo / PI + specular) * radiance * max(dot(normal, to_light), 0.0);

   return lighting; 
}

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
    vec4 albedo = texture(texture_sampler[fragment_input.texture_index], fragment_input.texture_coordinate) * fragment_input.color;
    color.a = albedo.a;
    if(color.a <= 0.01)
        discard;

    //TODO: textures mapping (normal, roughness, metallic)
    vec3 normal = fragment_input.normal;

    if(normal == vec3(0))
    {
        color = albedo;
        return;
    }
    
    if(albedo.rgb == vec3(0))
        return;
    
    normal = normalize(normal);

    float metallic = METALLIC;

    vec3 to_camera = normalize(global_uniform.camera_position - fragment_input.world_position.xyz); //V
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    
    vec3 ambient = vec3(AMBIENT) * albedo.rgb; // * ao
    vec3 total_light = vec3(0);
    
    for(uint i = 0; i < global_uniform.light_count; ++i)
    {
        Light light = global_uniform.lights[i];
        if(light.color == vec3(0))
            continue;
    
        total_light += PBR(light, normal, to_camera, albedo.rgb, metallic, ROUGHNESS, F0);
    }
    color = vec4(total_light + ambient.rgb, albedo.a);
    color.rgb = aces(color.rgb);
    //color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
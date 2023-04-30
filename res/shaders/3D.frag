//TODO: Remove hardcoded materials
#define AMBIENT 0.1
#define METALLIC 0.0
#define ROUGHNESS 0.3

#define PI 3.1415926535897932384626433832795

layout(location = 0) in VertexOutput 
{
    vec3 world_position;
    vec2 uv;
    vec3 normal;
    vec4 color;
    flat uint instance_image_index;
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

vec3 PBR(Light light, vec3 normal, vec3 to_camera, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 world_position)
{
   vec3 lighting = vec3(0);

   vec3 to_light = normalize(light.position - world_position.xyz); //wi
   vec3 halfway = normalize(to_camera + to_light);
   float attenuation = 1.0;
   if(light.linear != 0 || light.quadratic != 0)
   {
       float distance_to_light = distance(light.position, world_position.xyz);
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

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
    mat4 projection_view2D;
    mat4 projection;
    mat4 view;
    vec3 camera_position;
    float time;
    vec3 camera_direction;
    float delta_time;
    uvec2 resolution;
    uint frame;
    uint padding;
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
    vec4 albedo = texture(images[fragment_input.instance_image_index + DIFFUSE_TEXTURE], fragment_input.uv) * fragment_input.color;
    color.a = albedo.a;
    if (color.a <= 0.01)
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
    
    vec3 to_camera = normalize(global_uniform.camera_position - fragment_input.world_position.xyz);
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    
    vec3 ambient = vec3(AMBIENT) * albedo.rgb; // * ao
    vec3 total_light = vec3(0);
    
    for(uint i = 0; i < MAX_LIGHTS; ++i)
    {
        Light light = global_uniform.lights[i];
        if (light.color == vec3(0))
            continue;
    
        total_light += PBR(light, normal, to_camera, albedo.rgb, metallic, ROUGHNESS, F0, fragment_input.world_position.xyz);
    }
    color.rgb = aces(total_light + ambient.rgb);
}
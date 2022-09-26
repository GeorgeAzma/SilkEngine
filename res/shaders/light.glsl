#include math

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
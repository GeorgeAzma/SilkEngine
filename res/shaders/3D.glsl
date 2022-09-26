#type vertex
layout(location = 0) in vec3 V_position;
layout(location = 1) in vec2 V_texture_coordinate;
layout(location = 2) in vec3 V_normal;
layout(location = 3) in vec4 V_color;

layout(location = 4) in mat4 I_transform;
layout(location = 8) in uint I_image_index;
layout(location = 9) in vec4 I_color;

layout(location = 0) out VertexOutput 
{
    vec2 V_texture_coordinate;
    vec3 V_normal;
    flat uint I_image_index;
    flat vec4 I_color;
    vec3 V_world_position;
    vec4 V_color;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    vertex_output.V_texture_coordinate = V_texture_coordinate;
    vertex_output.V_normal = mat3(transpose(inverse(I_transform))) * V_normal;
    vertex_output.I_image_index = I_image_index;
    vertex_output.I_color = I_color;
    vertex_output.V_color = V_color;
    const vec4 world_position = I_transform * vec4(V_position, 1.0);
    vertex_output.V_world_position = world_position.xyz;
    gl_Position = global_uniform.projection_view * world_position;
}

#type fragment
//TODO: Remove hardcoded materials
#define AMBIENT 0.1
#define METALLIC 0.0
#define ROUGHNESS 0.3

layout(location = 0) in VertexOutput 
{
    vec2 V_texture_coordinate;
    vec3 V_normal;
    flat uint I_image_index;
    flat vec4 I_color;
    vec3 V_world_position;
    vec4 V_color;
} fragment_input;

#include light

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
    vec4 albedo = texture(images[fragment_input.I_image_index + DIFFUSE_TEXTURE], fragment_input.V_texture_coordinate) * fragment_input.I_color * fragment_input.V_color;
    color.a = albedo.a;
    if (color.a <= 0.01)
        discard;
    
    //TODO: textures mapping (normal, roughness, metallic)
    vec3 normal = fragment_input.V_normal;
    
    if (normal == vec3(0) || albedo.rgb == vec3(0))
    {
        color.rgb = albedo.rgb;
        return;
    }
    
    normal = normalize(normal);
    
    float metallic = METALLIC;
    
    vec3 to_camera = normalize(global_uniform.camera_position - fragment_input.V_world_position.xyz); //V
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    
    vec3 ambient = vec3(AMBIENT) * albedo.rgb; // * ao
    vec3 total_light = vec3(0);
    
    for(uint i = 0; i < MAX_LIGHTS; ++i)
    {
        Light light = global_uniform.lights[i];
        if (light.color == vec3(0))
            continue;
    
        total_light += PBR(light, normal, to_camera, albedo.rgb, metallic, ROUGHNESS, F0, fragment_input.V_world_position.xyz);
    }
    color.rgb = aces(total_light + ambient.rgb);
}
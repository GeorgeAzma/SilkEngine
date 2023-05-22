layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv;
layout(location = 2) in vec3 vertex_normal;
layout(location = 3) in vec4 vertex_color;
layout(location = 4) in vec4 vertex_tangent;

layout(location = 5) in mat4 instance_transform;
layout(location = 9) in vec4 instance_color;
layout(location = 10) in int instance_image_index;
layout(location = 11) in float instance_metallic;
layout(location = 12) in float instance_roughness;
layout(location = 13) in vec3 instance_emissive;

layout(location = 0) out VertexOutput 
{
    vec3 world_position;
    vec2 uv;
    vec3 normal;
    vec4 color;
    vec3 tangent;
    vec3 bitangent;
    flat int instance_image_index;
    flat float metallic;
    flat float roughness;
    flat vec3 emissive;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    vertex_output.uv = vertex_uv;
    vertex_output.color = vertex_color * instance_color;
    vertex_output.normal  = normalize(vec3(instance_transform * vec4(vertex_normal, 0)));
    vertex_output.tangent = normalize(vec3(instance_transform * vec4(vertex_tangent.xyz, 0)));
    vertex_output.tangent = normalize(vertex_output.tangent - dot(vertex_output.tangent, vertex_output.normal) * vertex_output.normal); // Re-orthogonalize T with respect to N
    vertex_output.bitangent = cross(vertex_output.normal, vertex_output.tangent.xyz) * vertex_tangent.w;
    vertex_output.instance_image_index = instance_image_index;
    vertex_output.metallic = instance_metallic;
    vertex_output.roughness = instance_roughness;
    vertex_output.emissive = instance_emissive;

    const vec4 world_position = instance_transform * vec4(vertex_position, 1.0);
    vertex_output.world_position = world_position.xyz;
    gl_Position = global_uniform.projection_view * world_position;
}
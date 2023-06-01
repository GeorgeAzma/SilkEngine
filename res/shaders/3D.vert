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
} vert_out;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

void main()
{
    vert_out.uv = vertex_uv;
    vert_out.color = vertex_color * instance_color;
    vert_out.normal  = normalize(vec3(instance_transform * vec4(vertex_normal, 0)));
    vert_out.tangent = normalize(vec3(instance_transform * vec4(vertex_tangent.xyz, 0)));
    vert_out.tangent = normalize(vert_out.tangent - dot(vert_out.tangent, vert_out.normal) * vert_out.normal); // Re-orthogonalize T with respect to N
    vert_out.bitangent = cross(vert_out.normal, vert_out.tangent.xyz) * vertex_tangent.w;
    vert_out.instance_image_index = instance_image_index;
    vert_out.metallic = instance_metallic;
    vert_out.roughness = instance_roughness;
    vert_out.emissive = instance_emissive;

    const vec4 world_position = instance_transform * vec4(vertex_position, 1.0);
    vert_out.world_position = world_position.xyz;
    gl_Position = global_uniform.projection_view * world_position;
}
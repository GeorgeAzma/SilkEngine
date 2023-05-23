layout(location = 0) in uint vertex;

layout(location = 0) out VertexOutput 
{
    vec3 uv;
    vec3 light;
} vertex_output;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

layout(push_constant) uniform PushConstant
{
    ivec4 chunk_position;
    vec4 light_position;
    vec4 light_color;
};

const vec2 uvs[4] = vec2[4](
    vec2(1, 1),
    vec2(1, 0),
    vec2(0, 0),
    vec2(0, 1)
);

const vec3 normals[6] = vec3[6](
    vec3( 0,  1,  0),  
    vec3( 0, -1,  0), 
    vec3(-1,  0,  0), 
    vec3( 1,  0,  0), 
    vec3( 0,  0, -1), 
    vec3( 0,  0,  1)  
);

const uvec3 positions[24] = uvec3[24](
    uvec3(0, 1, 1),  
    uvec3(0, 1, 0), 
    uvec3(1, 1, 0), 
    uvec3(1, 1, 1),
    uvec3(0, 0, 0),  
    uvec3(0, 0, 1), 
    uvec3(1, 0, 1), 
    uvec3(1, 0, 0),
    uvec3(0, 1, 1),  
    uvec3(0, 0, 1), 
    uvec3(0, 0, 0), 
    uvec3(0, 1, 0),
    uvec3(1, 1, 0),  
    uvec3(1, 0, 0), 
    uvec3(1, 0, 1), 
    uvec3(1, 1, 1),
    uvec3(0, 1, 0),  
    uvec3(0, 0, 0), 
    uvec3(1, 0, 0), 
    uvec3(1, 1, 0),
    uvec3(1, 1, 1),  
    uvec3(1, 0, 1), 
    uvec3(0, 0, 1), 
    uvec3(0, 1, 1)
);

const uint indices[6] = uint[6]
(
    2, 1, 3,
	3, 1, 0
);

void main()
{
	const vec3 light2_position = vec3(-3000, 2000, 1000);
	const vec3 light3_position = vec3(1000, -1000, 2000);

    uint vert_id = (vertex >> 15) & 3;
    uint face_id = (vertex >> 17) & 7;
    uvec3 local_pos = uvec3(vertex & (X - 1), (vertex >> 5) & (Y - 1), (vertex >> 10) & (Z - 1)) + positions[face_id * 4 + vert_id];
    vec3 world_pos = vec3(local_pos) + vec3(chunk_position.xyz * ivec3(X, Y, Z));
    vertex_output.uv = vec3(uvs[vert_id], (vertex >> 20) & 255);
    vertex_output.light = vec3(0.07);
    vertex_output.light += max(dot(normalize(light_position.xyz - world_pos),  normals[face_id]), 0.0);
    vertex_output.light += max(dot(normalize(light2_position.xyz - world_pos), normals[face_id]), 0.0);
    vertex_output.light += max(dot(normalize(light3_position.xyz - world_pos), normals[face_id]), 0.0);
    vertex_output.light *= light_color.rgb;
    gl_Position = global_uniform.projection_view * vec4(world_pos, 1.0);
}
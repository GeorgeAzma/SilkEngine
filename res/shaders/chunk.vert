layout(location = 0) in uvec2 vertex;

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
    vec4 chunk_position;
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
    vec3( 0, -1,  0),  
    vec3( 0,  0, -1), 
    vec3(-1,  0,  0), 
    vec3( 1,  0,  0), 
    vec3( 0,  0,  1),  
    vec3( 0,  1,  0) 
);

const vec3 positions[24] = vec3[24](
    // Y-
    vec3(0, 0, 0),  
    vec3(0, 0, 1), 
    vec3(1, 0, 1), 
    vec3(1, 0, 0),
    // Z-
    vec3(0, 1, 0),  
    vec3(0, 0, 0), 
    vec3(1, 0, 0), 
    vec3(1, 1, 0),
    // X-
    vec3(0, 1, 1),  
    vec3(0, 0, 1), 
    vec3(0, 0, 0), 
    vec3(0, 1, 0),
    // X+
    vec3(1, 1, 0),  
    vec3(1, 0, 0), 
    vec3(1, 0, 1), 
    vec3(1, 1, 1),
    // Z+
    vec3(1, 1, 1),  
    vec3(1, 0, 1), 
    vec3(0, 0, 1), 
    vec3(0, 1, 1),
    // Y+
    vec3(0, 1, 1),  
    vec3(0, 1, 0), 
    vec3(1, 1, 0), 
    vec3(1, 1, 1)
);

const uint indices[6] = uint[6]
(
    2, 1, 3,
	3, 1, 0
);

void main()
{
    uint vert_id = (vertex.x >> 18) & 3;
    uint face_id = (vertex.x >> 20) & 7;

    vec3 local_pos = vec3(vertex.x & (SIZE - 1), (vertex.x >> 6) & (SIZE - 1), (vertex.x >> 12) & (SIZE - 1)) + positions[face_id * 4 + vert_id];
    vec3 world_pos = local_pos + chunk_position.xyz * vec3(SIZE);

    vertex_output.uv = vec3(uvs[vert_id], (vertex >> 23) & 255);
    
	const vec3 light2_position = vec3(-3000, 2000, 1000);
	const vec3 light3_position = vec3(1000, -1000, 2000);
    //float ao = float((vertex.x >> 28) & 3) / 3.0;
    vertex_output.light = vec3(0.07);
    vertex_output.light += max(dot(normalize(light_position.xyz - world_pos),  normals[face_id]), 0.0);
    vertex_output.light += max(dot(normalize(light2_position.xyz - world_pos), normals[face_id]), 0.0);
    vertex_output.light += max(dot(normalize(light3_position.xyz - world_pos), normals[face_id]), 0.0);
    vertex_output.light *= light_color.rgb;

    gl_Position = global_uniform.projection_view * vec4(world_pos, 1.0);
}
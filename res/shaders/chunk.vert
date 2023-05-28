layout(set = 1, binding = 0) buffer Vertices
{
    uvec2 vertices[];
};

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

void main()
{
    uvec2 vertex = vertices[gl_VertexIndex];
    uint vert_id = (vertex.x >> 15) & 3;
    uint face_id = (vertex.x >> 17) & 7;

    uint idx = vertex.x & (VOLUME - 1);
    vec3 local_pos = vec3(idx % SIZE, idx / AREA, idx % AREA / SIZE) + positions[face_id * 4 + vert_id];
    vec3 world_pos = local_pos + chunk_position.xyz * vec3(SIZE);

    vertex_output.uv = vec3(uvs[vert_id], (vertex >> 20) & 255);
    
    vec3 normal = cross(positions[face_id * 4 + 2] - positions[face_id * 4 + 0], positions[face_id * 4 + 1] - positions[face_id * 4 + 0]);
	const vec3 light2_position = vec3(-300000, 200000, 100000);
	const vec3 light3_position = vec3(100000, -100000, 200000);
    float ao = 1.0 - float((vertex.x >> 28) & 3) / 3.0;
    vertex_output.light = vec3(0.07);
    vertex_output.light += max(dot(normalize(light_position.xyz),  normal), 0.0);
    vertex_output.light += max(dot(normalize(light2_position.xyz), normal), 0.0);
    vertex_output.light += max(dot(normalize(light3_position.xyz), normal), 0.0);
    vertex_output.light *= light_color.rgb;
    //vertex_output.light = max(normal, vec3(0)) * 0.5 + abs(normal) * 0.5;
    vertex_output.light *= ao;
 
    gl_Position = global_uniform.projection_view * vec4(world_pos, 1.0);
}
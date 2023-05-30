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
    const uint vert_id = (vertex.x >> 15) & 3;
    const uint face_id = (vertex.x >> 17) & 7;
    const uint idx = vertex.x & (VOLUME - 1);

    const vec3 local_pos = vec3(idx % SIZE, idx / AREA, idx % AREA / SIZE) + positions[face_id * 4 + vert_id];
    const vec3 world_pos = local_pos + chunk_position.xyz * vec3(SIZE);

    vertex_output.uv = vec3(uvs[vert_id], (vertex.x >> 20) & 255);
    
    const vec3 normal = cross(positions[face_id * 4 + 2] - positions[face_id * 4 + 0], positions[face_id * 4 + 1] - positions[face_id * 4 + 0]);
    const float ao = float((vertex.x >> 28) & 3) * 0.3333;
    vertex_output.light = vec3(0.07);
    vertex_output.light += max(dot(normalize(light_position.xyz),  normal), 0.0);
    vertex_output.light += max(dot(vec3(-0.8017, 0.5345, 0.2672), normal), 0.0);
    vertex_output.light += max(dot(vec3(0.4082, -0.4082, 0.8164), normal), 0.0);
    vertex_output.light *= light_color.rgb * (ao * 0.75 + 0.25);
    //vertex_output.light = max(normal, vec3(0)) * 0.5 + abs(normal) * 0.5;
 
    gl_Position = global_uniform.projection_view * vec4(world_pos, 1.0);
}
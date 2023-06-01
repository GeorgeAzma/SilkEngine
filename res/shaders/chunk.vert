layout(location = 0) in uvec2 vertex;

layout(location = 0) out VertexOutput 
{
    vec3 uv;
    vec3 light;
} vert_out;

layout(set = 0, binding = 0) uniform GlobalUniform
{
    mat4 projection_view;
} global_uniform;

layout(push_constant) uniform PushConstant
{
    ivec3 chunk_position;
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

const float face_light_values[6] = float[6](0.3, 0.4, 0.6, 0.5, 0.8, 1.0);

void main()
{
    const uint vert_id = (vertex.x) & 3;
    const uint face_id = (vertex.x >> 2) & 7;
    const uint idx = (vertex.x >> 5) & (VOLUME - 1);
    const float ao = float(vertex.y & 3) * 0.3333;
    vert_out.light = vec3(0.07) + face_light_values[face_id] * ao;
    vert_out.uv = vec3(uvs[vert_id], (vertex.x >> 23) & 255);
    const vec3 world_pos = vec3(ivec3(idx % SIZE, idx / AREA, idx % AREA / SIZE) + positions[face_id * 4 + vert_id] + chunk_position * DIM);
    gl_Position = global_uniform.projection_view * vec4(world_pos, 1.0);
}
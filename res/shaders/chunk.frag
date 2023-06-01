layout(location = 0) in VertexOutput 
{
    vec3 uv;
    vec3 light;
} frag_in;

layout(set = 0, binding = 1) uniform sampler2DArray texture_atlas;

layout(location = 0) out vec4 color;

void main()
{
    color = texture(texture_atlas, frag_in.uv) * vec4(frag_in.light, 1.0);
    if (color.a < 0.01)
        discard;
}
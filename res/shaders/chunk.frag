layout(location = 0) in FragmentInput 
{
    vec3 uv;
    vec3 light;
} fragment_input;

layout(set = 0, binding = 1) uniform sampler2DArray texture_atlas;

layout(location = 0) out vec4 color;

void main()
{
    color = texelFetch(texture_atlas, ivec3(textureSize(texture_atlas, 0).xy * fragment_input.uv.xy, fragment_input.uv.z), 0);
    if (color.a < 0.01)
        discard;

    color.rgb *= fragment_input.light;
}
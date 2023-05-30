layout(location = 0) in FragmentInput 
{
    vec3 uv;
    vec3 light;
} fragment_input;

layout(set = 0, binding = 1) uniform sampler2DArray texture_atlas;

layout(location = 0) out vec4 color;

void main()
{
    color = texture(texture_atlas, fragment_input.uv) * vec4(fragment_input.light, 1.0);
    if (color.a < 0.01)
        discard;
}
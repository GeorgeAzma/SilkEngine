layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat uint instance_image_index;
    flat vec4 instance_color;
    vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{
    color.a = texture(images[fragment_input.instance_image_index], fragment_input.texture_coordinate).r * fragment_input.instance_color.a * fragment_input.color.a;
    if(color.a <= 0.01)
        discard;
    color.rgb = fragment_input.instance_color.rgb * fragment_input.color.rgb;
}
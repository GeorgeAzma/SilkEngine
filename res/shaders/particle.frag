layout(location = 0) in VertexOutput 
{
    vec2 texture_coordinate;
    flat uint instance_image_index;
    vec4 color;
} fragment_input;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{    
    color = fragment_input.color;
    if (fragment_input.instance_image_index != 0)
        color *= texture(images[fragment_input.instance_image_index], fragment_input.texture_coordinate);
    if (color.a <= 0.01)
        discard;
}
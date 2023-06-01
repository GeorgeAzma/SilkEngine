layout(location = 0) in VertexOutput 
{
    vec2 uv;
    vec4 color;
    flat int instance_image_index;
} frag_in;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 0) uniform sampler2D images[MAX_IMAGE_SLOTS];

void main()
{    
    color = frag_in.color;
    if (frag_in.instance_image_index > -1)
        color *= texture(images[frag_in.instance_image_index], frag_in.uv);
    if (color.a <= 0.01)
        discard;
}
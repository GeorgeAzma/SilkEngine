layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform sampler2D image;

void main()
{
	color = texture(image, uv);
}
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput attachment;

layout(location = 0) out vec4 color;

void main()
{
	color = subpassLoad(attachment);
}
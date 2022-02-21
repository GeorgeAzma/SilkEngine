#type compute
#version 450
layout(local_size_x = 64) in;

layout(binding = 0) buffer image_buffer
{
	vec4 channels[];
};


void main()
{
	uint i = gl_GlobalInvocationID.x;
	channels[i].rgb = channels[i].bgr;
}
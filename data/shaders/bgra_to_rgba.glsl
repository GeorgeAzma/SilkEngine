#type compute
#version 450
layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 0, rgba8) uniform image2D image;


void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(image).xy;
	if(uv.x < size.x && uv.y < size.y)
	{
		vec4 col = imageLoad(image, uv);
		imageStore(image, uv, col.bgra);
	}
}
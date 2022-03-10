#type compute
layout(local_size_x = 64) in;

layout(set = 0, binding = 0) buffer Image
{
	uint colors[];
};


void main()
{
	uint i = gl_GlobalInvocationID.x;
	if(i < colors.length())
	{
		vec4 col = unpackUnorm4x8(colors[i]);
		colors[i] = packUnorm4x8(col.bgra);
	}
}
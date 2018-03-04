#version 450 core

layout (location = 0) out vec3 color;

layout (location = 0) in vec2 uv;

void main()
{
	vec2 uv2 = uv;
	for (int i = 0; i < 10000; i++)
	{
		uv2 *= 1.00001;
		uv2 *= 0.999998;
	}

	color = vec3(uv2, 0.0);
}
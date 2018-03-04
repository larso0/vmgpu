#version 450 core

layout (location = 0) out vec3 color;

layout (location = 0) in vec2 uv;

void main()
{
	color = vec3(uv, 0.0);
}
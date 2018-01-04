#version 450 core

layout (location = 0) out vec3 color;

layout (location = 0) in vec3 f_normal;

void main()
{
	float intensity = clamp(dot(normalize(vec3(-1.0, 1.0, 1.0)), f_normal), 0, 1) * 0.85;
	color = vec3(1.0, 0.0, 0.0) * (0.15 + intensity);
}
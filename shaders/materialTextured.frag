#version 450 core

layout (location = 0) out vec3 color;

layout (location = 0) in vec3 f_normal;
layout (location = 1) in vec2 f_uv;

layout (set = 0, binding = 0) uniform Material
{
	vec4 ambient, diffuse;
} material;
layout (set = 0, binding = 1) uniform sampler2D m_sampler;

void main()
{
	float intensity = clamp(dot(normalize(vec3(-1.0, 1.0, 1.0)), f_normal), 0, 1);
	color = max(material.ambient, material.diffuse * texture(m_sampler, f_uv) * intensity).xyz;
}
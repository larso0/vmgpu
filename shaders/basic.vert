#version 450 core

layout (location = 0) out vec3 f_normal;

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;

layout (push_constant) uniform _matrices
{
	mat4 mvp;
	mat4 normal;
} matrices;

void main()
{
	gl_Position = matrices.mvp * vec4(v_pos, 1.0);
	gl_Position.y = -gl_Position.y;
	f_normal = normalize(mat3(matrices.normal) * v_normal);
}
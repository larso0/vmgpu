#version 450 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

layout (location = 0) out vec3 f_normal;
layout (location = 1) out vec2 f_uv;

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
	f_uv = v_uv;
}
#version 450 core

layout (location = 0) out vec2 f_uv;

layout (push_constant) uniform PushConstants
{
	vec2 offset, extent;
} area;

void main()
{
	vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);
	f_uv = uv * area.extent + area.offset;
}
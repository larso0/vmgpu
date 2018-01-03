#version 450 core

vec2 vertices[4] = vec2[](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0)
);

layout (push_constant) uniform _Area
{
	vec2 offset, extent;
} area;

layout (location = 0) out vec2 textureCoordinate;

void main()
{
	gl_Position = vec4(area.extent * vertices[gl_VertexIndex] + area.offset, 0.0, 1.0);
	textureCoordinate = vertices[gl_VertexIndex];
}
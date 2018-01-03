#version 450 core

layout (location = 0) out vec2 textureCoordinate;

void main()
{
	textureCoordinate = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(textureCoordinate * 2.0f + -1.0f, 0.0f, 1.0f);
}
#version 450 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 textureCoordinate;

layout (set = 0, binding = 0) uniform sampler2D imageSampler;

void main()
{
	color = texture(imageSampler, textureCoordinate);
}
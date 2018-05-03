#version 450 core

layout (triangle_strip, max_vertices = 3) out;
layout (location = 0) out vec3 f_normal;
layout (location = 1) out vec2 f_uv;

layout (location = 0) in vec2 g_uvs[3];
layout (triangles) in;

layout (push_constant) uniform _matrices
{
	mat4 mvp;
	mat4 normal;
} matrices;

void main()
{
	f_normal = mat3(matrices.normal)
		 * normalize(cross(gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz,
			     gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz));
	for (uint i = 0; i < 3; i++)
	{
		gl_Position = matrices.mvp * gl_in[i].gl_Position;
		gl_Position.y = -gl_Position.y;
		f_uv = g_uvs[i];
		EmitVertex();
	}

        EndPrimitive();
}
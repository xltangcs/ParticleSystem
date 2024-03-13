#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in mat4 aInstanceMatrix;

out vec2 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = u_Projection * u_View * aInstanceMatrix * vec4(aPosition, 1.0);
	//gl_Position = vec4(a_Position, 1.0);
}

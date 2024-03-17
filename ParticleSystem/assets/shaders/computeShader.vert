#version 450 core

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	TexCoords = aTexCoords.xy;
	gl_Position = u_Projection * u_View * aPosition;
}
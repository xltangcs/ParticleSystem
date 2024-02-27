#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 u_Color;
uniform sampler2D texture1;

void main()
{
	vec4 texColor = texture(texture1, TexCoords);
	if(texColor.a < 0.1)
        discard;
	FragColor = texColor;
}

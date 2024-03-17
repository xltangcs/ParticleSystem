#version 450 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D snowTexture;

void main()
{
	vec4 texColor = texture(snowTexture, TexCoords);
	if(texColor.a < 0.1)
        discard;
	FragColor = texColor;
}

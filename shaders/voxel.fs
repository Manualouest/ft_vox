#version 330 core

out vec4 FragColor;

uniform sampler2D uTex;

in vec2 texCoord;
flat in float blockType;

void main()
{
	if (blockType == 0.0f)
		FragColor = vec4(0.4f, 1.0f, 0.4f, 1.0f);
	else
		FragColor = vec4(0.0f, 0.4f, 1.0f, 0.5f);
	// FragColor = texture(uTex, texCoord);
}

#version 330 core

out vec4 FragColor;

in vec2		TexCoords;
in vec2		FragPos;

uniform sampler2D screenTexture;

void main()
{
	int	zoomFactor = 100;
    FragColor = vec4(texture(screenTexture, gl_FragCoord.xy / zoomFactor).rgb / 2, 1.0);
}

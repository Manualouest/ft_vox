#version 330 core

out vec4 FragColor;

in vec2 fragPos;
in vec4 clipSpace;

uniform sampler2D screenTexture;
uniform sampler2D crossHairTexture;

void main()
{
	vec3 crossHairColor = texture(crossHairTexture, fragPos).rgb;
	if (crossHairColor.r == 0 && crossHairColor.g == 0 && crossHairColor.b == 0)
		discard ;

	vec3 ndc = clipSpace.xyz / clipSpace.w;
	ndc.xy = ndc.xy / 2 + 0.5;
	vec3	color = 1 - texture(screenTexture, ndc.xy).rgb;
    FragColor = vec4(color, 1.0);
}

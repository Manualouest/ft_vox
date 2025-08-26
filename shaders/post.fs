#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec2 FragPos;

uniform	vec3	viewPos;
uniform	float	time;

uniform sampler2D screenTexture;
uniform sampler2D depthTex;

float   water_level = 15.0;

float VignetteIntensity = 0.25;
float VignetteRadius = 0.75;
uniform float	RENDER_DISTANCE;

uniform	float SCREEN_WIDTH;
uniform	float SCREEN_HEIGHT;

uniform	bool	blur;
uniform	bool	underwater;

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (((2.0 * near * far) / (far + near - z * (far - near)) - near) / (far - near));
}

vec3	blurImage(int kernelSize, sampler2D tex)
{
	vec3	color;
	int halfKernel = kernelSize / 2;
	float pixelX = 1.0 / SCREEN_WIDTH;
	float pixelY = 1.0 / SCREEN_HEIGHT;
	vec2 uv = TexCoords;

	for (int x = -halfKernel; x < halfKernel; x++)
	{
		for (int y = -halfKernel; y < halfKernel; y++)
		{
			vec2 offset = vec2(float(x) * pixelX, float(y) * pixelY);
			color += texture(tex, uv + offset).rgb;
		}
	}

	color /= float(kernelSize * kernelSize);
	return (color);
}

void main()
{
	vec3 color = vec3(0.0);
	vec2 uv = TexCoords;

	if (blur)
		color = blurImage(16, screenTexture);
	else
		color = texture(screenTexture, uv).rgb;

	if (underwater && !blur)
		color = blurImage(4, screenTexture);

	vec2 centeredUV = uv - vec2(0.5);
	float dist = length(centeredUV);
	float vignette = smoothstep(VignetteRadius, VignetteRadius - 0.3, dist);
	vignette = mix(1.0, vignette, VignetteIntensity);

	if (underwater)
		color *= vec3(0.3, 0.8, 0.87);

	color = clamp(color, 0.0, 1.0);
	FragColor = vec4(color * vignette, 1.0);
}

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

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (((2.0 * near * far) / (far + near - z * (far - near)) - near) / (far - near));
}

void main()
{
	vec2 uv = TexCoords;

	float pixelX = 1.0 / SCREEN_WIDTH;
	float pixelY = 1.0 / SCREEN_HEIGHT;

	vec3 color = vec3(0.0);

	if (blur)
	{
		int kernelSize = 16;
		int halfKernel = kernelSize / 2;

		for (int x = -halfKernel; x < halfKernel; x++)
		{
			for (int y = -halfKernel; y < halfKernel; y++)
			{
				vec2 offset = vec2(float(x) * pixelX, float(y) * pixelY);
				color += texture(screenTexture, uv + offset).rgb;
			}
		}

		color /= float(kernelSize * kernelSize);
	}
	else
		color = texture(screenTexture, uv).rgb;


	vec2 centeredUV = uv - vec2(0.5);
	float dist = length(centeredUV);
	float vignette = smoothstep(VignetteRadius, VignetteRadius - 0.3, dist);
	vignette = mix(1.0, vignette, VignetteIntensity);

	color = clamp(color, 0.0, 1.0);
	FragColor = vec4(color * vignette, 1.0);
}

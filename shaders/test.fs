#version 330 core

out vec4 FragColor;

in vec2    TexPos;
in vec2    FragPos;

uniform sampler2D   screenTexture;

vec3 ditheredQuantize(vec2 TexPos, vec3 color, int levels) {
	// 4x4 Bayer matrix (values normalized to [0, 1])
	float bayer[16] = float[](
		0.0 / 16.0,		8.0 / 16.0,		2.0 / 16.0,		10.0 / 16.0,
		12.0 / 16.0,	4.0 / 16.0,		14.0 / 16.0,	6.0 / 16.0,
		3.0 / 16.0,		11.0 / 16.0,	1.0 / 16.0,		9.0 / 16.0,
		15.0 / 16.0,	7.0 / 16,		13.0 / 16.0,	5.0 / 16.0
	);

	// Get 4x4 index from fragment coordinates
	int x = int(mod(TexPos.x, 4.0));
	int y = int(mod(TexPos.y, 4.0));
	float threshold = bayer[y * 4 + x];

	// Apply threshold as offset before quantizing
	vec3 dithered = color + (1.0 / float(levels)) * (threshold - 0.5);
	return (floor(clamp(dithered, 0.0, 1.0) * float(levels)) / float(levels));
}

float   near = 0.1;
float   far = 448;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	// vec3 color = texture(screenTexture, TexPos).rgb;
	vec3 color = vec3(LinearizeDepth(texture(screenTexture, TexPos).r) - near) / (far - near);

    // color = ditheredQuantize(TexPos, color, 8);

    vec3 black = vec3(0.0);

    if (abs(FragPos.x) > 0.97 || abs(FragPos.y) > 0.97) //Draws the outline on the last 3% of the texture
        color = black;

    vec2 centeredUV = vec2(TexPos.x - 0.5, TexPos.y + 0.5);
    float dist = length(centeredUV);

    float VignetteIntensity = 1;
    float VignetteRadius = 0.70;

    float vignette = smoothstep(VignetteRadius, VignetteRadius - 0.3, dist);
    vignette = mix(1.0, vignette, VignetteIntensity);

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(color * vignette, 1.0);
}

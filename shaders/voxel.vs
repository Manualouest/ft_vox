#version 330 core

layout (location = 0) in uint data;
vec3	aPos = vec3(float(data & 63u), float((data >> 6) & 511u), float((data >> 15) & 63u));
vec2	aTex = vec2(float((data >> 21) & 1u), float((data >> 22) & 1u));
float	aType = (data >> 23) & 63u;
uint	aNormalId = (data >> 29) & 7u;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3		Normal;
out vec3		FragPos;
out vec4		WorldPos;
out vec2		texCoord;
flat out float	blockType;
out vec4		clipSpace;

uniform float	time;

vec3 Normals[6] = vec3[](
	vec3 (-1, 0, 0),
	vec3 (1, 0, 0),
	vec3 (0, 1, 0),
	vec3 (0, 0, 1),
	vec3 (0, 0, -1),
	vec3 (0, -1, 0)
);

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f); // Smooth interpolation

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < 5; i++)
    {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float   amplitude = 1.0;
float   scale = 0.016;

float   animSpeed = 20;

float height(vec2 pos)
{
    return (clamp(fbm(pos * scale + (time / animSpeed)) * amplitude, 0.0, 1.0));
}

vec3 calculateNormal(vec2 pos)
{
    float delta = 0.01;

    float hL = height(pos - vec2(delta, 0.0));
    float hR = height(pos + vec2(delta, 0.0));
    float hD = height(pos - vec2(0.0, delta));
    float hU = height(pos + vec2(0.0, delta));

    float dx = hR - hL;
    float dz = hU - hD;

    return normalize(vec3(-dx, 2.0 * delta, -dz));
}

void main()
{
	Normal = Normals[aNormalId];

	if ((Normal.x == 1 || Normal.x == -1) && Normal.y == 0 && Normal.z == 0)
		texCoord = aPos.zy;
	else if (Normal.x == 0 && (Normal.y == 1 || Normal.y == -1) && Normal.z == 0)
		texCoord = aPos.xz;
	else if (Normal.x == 0 && Normal.y == 0 && (Normal.z == 1 || Normal.z == -1))
		texCoord = aPos.xy;
	blockType = aType;

	FragPos = aPos;

	if (int(blockType) == 0)
	{
		vec4	tmpWorldPos = model * vec4(FragPos, 1.0);
		FragPos -= (vec3(0, height(tmpWorldPos.xz), 0) / 8) * 2;
		Normal = calculateNormal(tmpWorldPos.xz);
	}

	WorldPos = model * vec4(FragPos, 1.0);

	gl_Position = projection * view * WorldPos;
	clipSpace = gl_Position;
}

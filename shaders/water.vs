#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4    view;
uniform mat4    projection;
uniform mat4    model;

out vec3    FragPos;

uniform float time;

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

float   amplitude = 5.0;
float   scale = 0.016;

float   animSpeed = 50;

float height(vec2 pos)
{
    return fbm(pos * scale + (time / animSpeed)) * amplitude;
}

float   water_level = 15.0;

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

out vec3    Normal;

out vec4 worldPos;

out vec4 clipSpacePos;

void main()
{
    vec3 pos = aPos;

    pos.y = water_level;

    vec4 worldPosXZ = model * vec4(pos.x, 0.0, pos.z, 1.0);
    vec2 worldXZ = worldPosXZ.xz;

    float n = fbm(worldXZ * scale + (time / animSpeed));
    pos.y += n * amplitude;

    worldPos = model * vec4(pos, 1.0);
    FragPos = worldPos.xyz;

    vec3 normal = calculateNormal(worldXZ);
    Normal = mat3(transpose(inverse(model))) * normal;

    gl_Position = projection * view * worldPos;
    clipSpacePos = gl_Position;
}

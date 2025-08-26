#version 330 core

out vec4 FragColor;

uniform sampler2D terrainDepthTex;
uniform sampler2D waterDepthTex;
uniform vec3 lightAmbient;
uniform vec3 sunLightDirection;
uniform vec3 sunLightDiffuse;
uniform vec3 moonLightDirection;
uniform vec3 moonLightDiffuse;

in vec2 texCoord;
flat in float blockType;
in vec3	Normal;
in vec3	FragPos;
in vec4	WorldPos;

uniform float RENDER_DISTANCE;
float FOG_DISTANCE = RENDER_DISTANCE - (RENDER_DISTANCE / 2);

uniform vec3 viewPos;
in vec4	clipSpace;

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (((2.0 * near * far) / (far + near - z * (far - near)) - near) / (far - near));
}

uniform bool getDepth;

const vec3  SHORE_COLOR = vec3(0.3, 0.8, 0.87);
const vec3  DEEP_COLOR = vec3(0.0, 0.2, 1.0);
uniform vec3  FOG_COLOR;

uniform	sampler2D textureAtlas;

vec3	getBlockTexture(int ID)
{
	//To optimize on chunk data we dont give the block's UV's instead we give it the world pos of the vertice (see voxel.vs) so I have to convert it to actual UV
	vec2	baseUV = texCoord;
	baseUV = fract(baseUV);

	int row = 15 - (ID / 16);;
    int col = ID % 16;

    vec2 cellSize = vec2(16.0 / 256.0);
    vec2 atlasOffset = vec2(col, row) * cellSize;
    vec2 atlasUV = atlasOffset + baseUV * cellSize;

	return (texture(textureAtlas, atlasUV).rgb);
}

void main()
{
	float   shininess = 64.0f;
    float   actualShiness = 0.1;
	float	alpha = 1.0;

	vec3	color;
	float	dist = length(WorldPos.xyz - viewPos) / FOG_DISTANCE;
	dist = clamp(pow(dist, 1.5), 0.0, 1.0);

	color = getBlockTexture(int(blockType));
	if (color.r == 1 && color.g == 0 && color.b == 1)
		discard ;

	if (int(blockType) == 0) //WATER
	{
		shininess = 256.0f;
    	actualShiness = 1.0;
		alpha = 0.8;
		vec3 ndc = clipSpace.xyz / clipSpace.w;
	    ndc.xy = ndc.xy / 2 + 0.5;

	    color = SHORE_COLOR;
		color *= getBlockTexture(0);
	    color = clamp(color, 0, 1);
	}

	//DIRECTIONAL LIGHT
    vec3    viewDir = normalize(viewPos - WorldPos.xyz);
    vec3    lightSpecular = (sunLightDiffuse + moonLightDiffuse) * 1.05;

    vec3 ambient = lightAmbient * color;

    vec3 norm = normalize(Normal);
    vec3 sunLightDir = normalize(-sunLightDirection);
    float sunDiff = max(dot(norm, sunLightDir), 0.0);
    vec3 moonLightDir = normalize(-moonLightDirection);
    float moonDiff = max(dot(norm, moonLightDir), 0.0);
    vec3 diffuse = (sunLightDiffuse * sunDiff + moonLightDiffuse * moonDiff) * color;

	vec3 reflectDir = vec3(0);
	if (sunLightDirection.y < moonLightDirection.y)
    	reflectDir = reflect(-sunLightDir, norm);
	else
    	reflectDir = reflect(-moonLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * spec * actualShiness;

    vec3 result = ambient + diffuse + specular;
    //DIRECTIONAL LIGHT

	result = mix(result, FOG_COLOR, dist);

	FragColor = vec4(result, alpha + (specular.r / 2));
}

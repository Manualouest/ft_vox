#version 330 core

out vec4 FragColor;

uniform sampler2D stoneTexture;
uniform sampler2D dirtTexture;
uniform sampler2D grassTexture;
uniform sampler2D sandTexture;
uniform sampler2D grassSideTexture;
uniform sampler2D waterTexture;
uniform sampler2D missingTexture;

uniform sampler2D terrainDepthTex;
uniform sampler2D waterDepthTex;

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
const vec3  FOG_COLOR = vec3(0.6, 0.8, 1.0);

uniform sampler2D sandstoneTexture;
uniform sampler2D redSandstoneTexture;
uniform sampler2D terracottaTexture;
uniform sampler2D snowTexture;
uniform sampler2D redSandTexture;

uniform sampler2D redTerracottaTexture;
uniform sampler2D brownTerracottaTexture;
uniform sampler2D yellowTerracottaTexture;
uniform sampler2D lightGrayTerracottaTexture;
uniform sampler2D whiteTerracottaTexture;

vec3	getBlockTexture(int ID)
{
	if (ID == 1)
		return (texture(stoneTexture, texCoord).rgb);
	if (ID == 2)
		return (texture(dirtTexture, texCoord).rgb);
	if (ID == 3)
		return (texture(grassTexture, texCoord).rgb);
	if (ID == 4)
		return (texture(grassSideTexture, texCoord).rgb);
	if (ID == 5)
		return (texture(sandTexture, texCoord).rgb);
	if (ID == 7)
		return (texture(sandstoneTexture, texCoord).rgb);
	if (ID == 8)
		return (texture(terracottaTexture, texCoord).rgb);
	if (ID == 9)
		return (texture(redSandstoneTexture, texCoord).rgb);
	if (ID == 10)
		return (texture(snowTexture, texCoord).rgb);
	if (ID == 11)
		return (texture(redSandTexture, texCoord).rgb);
	if (ID == 12)
		return (texture(redTerracottaTexture, texCoord).rgb);
	if (ID == 13)
		return (texture(brownTerracottaTexture, texCoord).rgb);
	if (ID == 14)
		return (texture(yellowTerracottaTexture, texCoord).rgb);
	if (ID == 15)
		return (texture(lightGrayTerracottaTexture, texCoord).rgb);
	if (ID == 16)
		return (texture(whiteTerracottaTexture, texCoord).rgb);
	if (ID == 42)
		return (texture(missingTexture, texCoord).rgb);
	return (texture(grassTexture, texCoord).rgb);
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

	if (int(blockType) == 0) //WATER
	{
		shininess = 256.0f;
    	actualShiness = 1.0;
		alpha = 0.8;
		vec3 ndc = clipSpace.xyz / clipSpace.w;
	    ndc.xy = ndc.xy / 2 + 0.5;

	    color = SHORE_COLOR;
		color *= texture(waterTexture, texCoord).rgb;
	    color = clamp(color, 0, 1);
	}

	//DIRECTIONAL LIGHT
    vec3    viewDir = normalize(viewPos - WorldPos.xyz);
    vec3    lightDirection = vec3(-0.8f, -0.4f, -0.45f);
    vec3    lightAmbient = vec3(0.2f, 0.2f, 0.2f);
    vec3    lightDiffuse = vec3(1.0f, 1.0f, 1.0f);
    vec3    lightSpecular = vec3(1.0f, 1.0f, 1.0f);

    vec3 ambient = lightAmbient * color;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * diff * color;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * spec * actualShiness;

    vec3 result = ambient + diffuse + specular;
    //DIRECTIONAL LIGHT

	result = mix(result, FOG_COLOR, dist);

	FragColor = vec4(result, alpha + (specular.r / 2));
}

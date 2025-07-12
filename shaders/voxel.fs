#version 330 core

out vec4 FragColor;

uniform sampler2D uTex;

uniform sampler2D terrainDepthTex;
uniform sampler2D waterDepthTex;

in vec2 texCoord;
flat in float blockType;
flat in vec3	Normal;
in vec3	FragPos;
in vec4	WorldPos;

uniform float RENDER_DISTANCE;

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

void main()
{
	// vec3 Normal = cross(dFdx(FragPos), dFdy(FragPos));
	float   shininess = 256.0f;
    float   actualShiness = 1;
	float	alpha = 1.0;

	vec3	color;
	if (blockType == 0.0f)
	{
		shininess = 64;
		actualShiness = 0.1;
		color = abs(Normal);
		// color = texture(uTex, texCoord).rgb;
	}
	else
	{
		if (getDepth)
			discard ;
		alpha = 0.8;
		vec3 ndc = clipSpace.xyz / clipSpace.w;
	    ndc.xy = ndc.xy / 2 + 0.5;
	
	    //FIGURE OUT WATER COLOR BASED ON AMOUNT TRAVERSED
	
	    //Takes terrain depth value from texture
	    float terrainDepthValue = texture(terrainDepthTex, ndc.xy).r;
	    float distToTerrain = LinearizeDepth(terrainDepthValue, 0.1, RENDER_DISTANCE);
	
	    //Takes water depth value from texture
	    float waterDepthValue = texture(waterDepthTex, ndc.xy).r;
	    float distToWater = LinearizeDepth(waterDepthValue, 0.1, RENDER_DISTANCE);
	
	    //Gets the amount of water traversed (between terrain and surface of water)
	    float waterDepth = distToTerrain - distToWater;
	
	    color = mix(SHORE_COLOR, DEEP_COLOR, 1 - exp(-waterDepth * 20));
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

	FragColor = vec4(result, alpha + (specular.r / 2));
}

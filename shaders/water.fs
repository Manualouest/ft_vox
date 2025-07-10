#version 330 core

out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;

in vec4 worldPos;
in vec4 clipSpacePos;

uniform sampler2D terrainDepthTex;
uniform sampler2D waterDepthTex;
uniform float RENDER_DISTANCE;
float fogDistance = RENDER_DISTANCE - (RENDER_DISTANCE / 2);

const vec3  FOG_COLOR = vec3(0.6, 0.8, 1.0);
const vec3  SHORE_COLOR = vec3(0.3, 0.8, 0.87);
const vec3  DEEP_COLOR = vec3(0.0, 0.2, 1.0); 

float LinearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (((2.0 * near * far) / (far + near - z * (far - near)) - near) / (far - near));
}

void main()
{
    vec3 color = vec3(0.0);

    //Gets screen pos of current pixel to then pick depth
    vec3 ndc = clipSpacePos.xyz / clipSpacePos.w;
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
    //FIGURE OUT WATER COLOR BASED ON AMOUNT TRAVERSED

    //DIRECTIONAL LIGHT
    float   shininess = 256.0f;
    float   actualShiness = 1;

    vec3    viewDir = normalize(viewPos - FragPos);
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

    // APPLY FOG
    result = mix(result, FOG_COLOR, distToWater);
    result = clamp(result, 0.0, 1.0);

    //Water is opaque when light shines on it
    float   alpha = 0.8 + (specular.r / 2);

    FragColor = vec4(result, alpha);
}

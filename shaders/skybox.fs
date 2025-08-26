#version 330 core

out vec4 FragColor;

in vec3 TexCoords;
in vec3 FragPos;

uniform samplerCube skybox;
uniform vec3  FOG_UP_COLOR;
uniform vec3  FOG_COLOR;

void main()
{
    float   height = clamp(normalize(FragPos).y, 0.0, 1.0);
    vec3    color;
    if (height > 0.1 && height < 0.25)
        color = mix(FOG_COLOR, FOG_UP_COLOR, clamp((height - 0.175) * 20, 0.0, 1.0));
    else if (height > 0.25)
    {
        color = FOG_UP_COLOR;
    }
    else
        color = FOG_COLOR;
    FragColor = vec4(color, 1.0);
}

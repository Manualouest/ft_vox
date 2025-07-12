#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aType;
layout (location = 2) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

flat out vec3    Normal;
out vec3    FragPos;
out vec4    WorldPos;
out vec2 texCoord;
flat out float blockType;
out vec4    clipSpace;


void main()
{
    Normal = aNormal;
    
    if ((Normal.x == 1 || Normal.x == -1) && Normal.y == 0 && Normal.z == 0)
        texCoord = aPos.zy;
    if (Normal.x == 0 && (Normal.y == 1 || Normal.y == -1) && Normal.z == 0)
        texCoord = aPos.xz;
    if (Normal.x == 0 && Normal.y == 0 && (Normal.z == 1 || Normal.z == -1))
        texCoord = aPos.xy;

    blockType = aType;
    FragPos = aPos;

    WorldPos = model * vec4(aPos, 1.0);

    gl_Position = projection * view * WorldPos;
    clipSpace = gl_Position;
}

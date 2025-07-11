#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in float aType;
layout (location = 3) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3	Normal;
out vec3	FragPos;
out vec4	WorldPos;
out vec2 texCoord;
flat out float blockType;
out vec4	clipSpace;

void main()
{
	Normal = aNormal;
	texCoord = aTex;
	blockType = aType;
	FragPos = aPos;

	WorldPos = model * vec4(aPos, 1.0);

	gl_Position = projection * view * WorldPos;
	clipSpace = gl_Position;
}

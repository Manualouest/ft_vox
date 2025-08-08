#version 330 core


// for old uncomment this and commet the next one
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aTex;
// layout (location = 2) in float aType;
// layout (location = 3) in vec3 aNormal;

layout (location = 0) in uint data;
vec3	aPos = vec3(float(data & 63u), float((data >> 6) & 511u), float((data >> 15) & 63u));
vec2	aTex = vec2(float((data >> 21) & 1u), float((data >> 22) & 1u));
float	aType = (data >> 23) & 7u;
uint	aNormalId = (data >> 26) & 7u;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3		Normal;
out vec3		FragPos;
out vec4		WorldPos;
out vec2		texCoord;
flat out float	blockType;
out vec4		clipSpace;


vec3 Normals[6] = vec3[](
	vec3 (-1, 0, 0),
	vec3 (1, 0, 0),
	vec3 (0, 1, 0),
	vec3 (0, 0, 1),
	vec3 (0, 0, -1),
	vec3 (0, -1, 0)
);

void main()
{
	//Normal = aNormal; // for old uncomment this and commet the next one
	Normal = Normals[aNormalId];

	if ((Normal.x == 1 || Normal.x == -1) && Normal.y == 0 && Normal.z == 0)
		texCoord = aPos.zy;
	else if (Normal.x == 0 && (Normal.y == 1 || Normal.y == -1) && Normal.z == 0)
		texCoord = aPos.xz;
	else if (Normal.x == 0 && Normal.y == 0 && (Normal.z == 1 || Normal.z == -1))
		texCoord = aPos.xy;
	blockType = aType;
	FragPos = aPos;

	WorldPos = model * vec4(aPos, 1.0);

	gl_Position = projection * view * WorldPos;
	clipSpace = gl_Position;
}

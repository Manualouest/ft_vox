#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    FragPos = aPos;
    TexCoords = aPos;
    gl_Position = projection * model * view * vec4(aPos, 1.0);
}

#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexPos;

uniform mat4    view;
uniform mat4    projection;
uniform mat4    model;

out vec2    FragPos;
out vec2    TexPos;

void main()
{
    TexPos = vec2(aTexPos.x, -aTexPos.y);
    FragPos = aPos;

	gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}

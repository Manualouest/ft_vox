#version 330 core

out vec4 FragColor;
uniform vec3 color;

in vec2 fragPos;

void main()
{
    FragColor = vec4(color.rgb, 1.0);
}

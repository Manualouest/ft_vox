#version 330 core

out vec4 FragColor;
uniform vec3 color;

in vec2 fragPos;
uniform sampler2D tex;

void main()
{
    vec3 color = texture(tex, fragPos).rgb;

    FragColor = vec4(color, 1.0);
}

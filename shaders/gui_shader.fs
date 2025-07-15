#version 330 core
out vec4 FragColor;

uniform sampler2D tex0;

in vec2 fragPos;

void main()
{
    vec4 texColor = texture(tex0, fragPos);
    if (texColor.a < 0.1)
        discard ;
    FragColor = texColor;
}

#version 330 core
out vec4 FragColor;

uniform sampler2D tex0;

in vec2 fragPos;
in vec4 clip;

uniform float time;
uniform float SCREEN_WIDTH;
uniform float SCREEN_HEIGHT;

void main() {
    vec4 texColor = texture(tex0, fragPos);
    if (texColor.r == 0, texColor.g == 0 && texColor.b == 0)
        discard ;
    FragColor = vec4(vec3(0.0, 1.0, 0.2), 1.0);
}

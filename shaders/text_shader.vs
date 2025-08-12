#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 projection;
uniform mat4 model;

out vec2 fragPos;
out vec4 clip;
out vec2 texPos;

uniform int charIndex;

void main() {
    fragPos = vec2(aPos.x, aPos.y);
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    clip = gl_Position;

    int row = charIndex / 16;
    int col = charIndex % 16;

    vec2 cellSize = vec2(1.0 / 16.0);
    vec2 atlasOffset = vec2(col, row) * cellSize;
    texPos = atlasOffset + aUV * cellSize;
}

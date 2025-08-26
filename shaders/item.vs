#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 projection;
uniform mat4 model;

out vec2 fragPos;

void main() {
    fragPos = vec2(aPos.x, -aPos.y);
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}

#version 450

layout(location = 0) out vec4 outColor;

vec3 fragColor = vec3(0.01, 0.3, 1.0);

void main() {
    outColor = vec4(fragColor, 1.0);
}
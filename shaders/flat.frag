#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 outColor;

layout(location =0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) flat in uint tid;

layout(binding = 2) uniform sampler2D mySampler[];

void main() {
    vec4 color = texture(mySampler[tid], fragUV);
    outColor = vec4(color.rgb, 1.0);
}
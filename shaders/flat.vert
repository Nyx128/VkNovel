#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) flat out uint tid;

struct Vertex
{
	float vx, vy, vz, ux, uy;
};

struct Sprite{
	mat4 transform;
	vec3 col;
	uint tex_index;
};

layout(binding = 0) readonly buffer Vertices
{
	Vertex vertices[];
};

layout(binding = 1) readonly buffer Sprites
{
	Sprite sprites[];
};

void main() {
    Vertex v = vertices[gl_VertexIndex];
	Sprite sp = sprites[gl_InstanceIndex];
    gl_Position = sp.transform * vec4(v.vx, v.vy, v.vz, 1.0);
	fragColor = sp.col;
	fragUV = vec2(v.ux, v.uy);
	tid = sp.tex_index;
}
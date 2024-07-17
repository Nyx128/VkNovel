#version 450

layout(location = 0) out vec3 fragColor;

struct Vertex
{
	float vx, vy, vz;
};

struct Sprite{
	mat4 transform;
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
}
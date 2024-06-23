#version 450

layout(location = 0) out vec3 fragColor;

struct Vertex
{
	float vx, vy, vz;
};

layout(binding = 0) readonly buffer Vertices
{
	Vertex vertices[];
};

void main() {
    Vertex v = vertices[gl_VertexIndex];
    gl_Position = vec4(v.vx, v.vy, v.vz, 1.0);
    fragColor = vec3(0.02, 0.3, 1.0);
}
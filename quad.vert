#version 450 core

in vec2 vertex;

out vec2 TexCoords;

uniform mat4 projection;

void main(void) {
	TexCoords = vertex;
	gl_Position = vec4(vertex.xy, 1.0, 1.0);
}


#version 450 core

out vec4 out_color;

in vec2 TexCoords;

uniform sampler2D Texture;

void main(void) {
	out_color = vec4(texture(Text_Texture, TexCoords).rgb,1.0);
}




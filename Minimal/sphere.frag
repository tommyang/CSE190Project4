#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D myTextureSampler;

void main(){
	// Output color = color of the texture at the specified UV
	color = vec3(texture(myTextureSampler, UV).r, texture(myTextureSampler, UV).g, texture(myTextureSampler, UV).b);
}

#version 330 core
// Andy Thai

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in vec3 TexCoords;
in vec3 Normal;

// You can output many things. The first vec4 type output determines the color of the fragment
out vec4 color;

// Uniform
uniform samplerCube skybox;
uniform DirLight dirLight;


void main()
{
    vec3 norm = normalize(Normal);
	vec3 l = normalize(dirLight.direction);
    color = texture(skybox, TexCoords);
}
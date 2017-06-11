#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;


struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};

in vec3 FragPos;
in vec3 Normal;
uniform Material material;
uniform vec3 viewPos;
uniform PointLight pointLights[2];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    //color = texture(texture_diffuse1, TexCoords);
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    for(int i=0; i<2; i++){
        result = result + CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    color = vec4(result, 1.0);
    //color = vec4(material.diffuse.r, material.diffuse.g, material.diffuse.b, 1.0f);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 ambientAndDiffuseMaterial = vec3(texture(texture_diffuse1,TexCoords));
    vec3 specularMaterial = vec3(texture(texture_specular1,TexCoords));

    // Ambient
    vec3 ambient = light.ambient *ambientAndDiffuseMaterial;

    // Diffuse shading
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * ambientAndDiffuseMaterial;

    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularMaterial;

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine results
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

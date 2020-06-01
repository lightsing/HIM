#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light lights[NR_POINT_LIGHTS];

uniform float far_plane;
uniform bool shadows;
uniform samplerCube depthMap;

float ShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - lights[0].position;
    // Use the fragment to light vector to sample from the depth map
    float closestDepth = texture(depthMap, fragToLight).r;
    // It is currently in linear range between [0,1]. Let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Now test for shadows
    float bias = 0.2; // We use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0, 0, 0);
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        Light light = lights[i];

        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

        // diffuse
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

        // specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));

        float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;

        result += (ambient + (diffuse + specular) * (1.0 - shadow)) * attenuation;
        // result += vec3(1.0);
    }

    FragColor = vec4(result, 1.0);
}
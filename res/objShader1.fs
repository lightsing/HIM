#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

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

uniform vec3 viewPos;
uniform Material material;
uniform Light lights[NR_POINT_LIGHTS];

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0, 0, 0);
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        Light light = lights[i];

        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, fs_in.TexCoords).rgb;

        // diffuse
        vec3 lightDir = normalize(light.position - fs_in.FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.TexCoords).rgb;

        // specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, fs_in.TexCoords).rgb;

        // calculate attenuation
        float distance = length(light.position - fs_in.FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));

        // calculate shadow
        float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

        result += (ambient + (diffuse + specular) * (1.0 - shadow) * attenuation;
    }

    FragColor = vec4(result, 1.0);
}
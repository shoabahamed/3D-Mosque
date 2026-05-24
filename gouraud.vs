#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 uvTiling;
uniform vec3 viewPos;
uniform sampler2D diffuseMap;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;
uniform float materialTexBlend;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 36
#define NR_SPOT_LIGHTS 4

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

// Toggle uniforms
uniform bool dirLightOn;
uniform bool pointLightsOn;
uniform bool spotLightsOn;
uniform bool ambientOn;
uniform bool diffuseOn;
uniform bool specularOn;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess);

void main()
{
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    vec2 TexCoord = aTexCoord * uvTiling;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 texColor = texture(diffuseMap, TexCoord).rgb;
    float blend = clamp(materialTexBlend, 0.0, 1.0);
    vec3 ambientBase = mix(texColor, material.ambient, blend);
    vec3 diffuseBase = mix(texColor, material.diffuse, blend);
    vec3 specularBase = mix(texColor, material.specular, blend);

    vec3 result = vec3(0.0);

    if (dirLightOn)
        result += CalcDirLight(dirLight, norm, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);

    if (pointLightsOn) {
        for (int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);
    }

    if (spotLightsOn) {
        for (int i = 0; i < NR_SPOT_LIGHTS; i++)
            result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);
    }

    vertexColor = result;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = ambientOn  ? light.ambient  * ambientBase : vec3(0.0);
    vec3 diffuse  = diffuseOn  ? light.diffuse  * diff * diffuseBase : vec3(0.0);
    vec3 specular = specularOn ? light.specular * spec * specularBase : vec3(0.0);
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = ambientOn  ? light.ambient  * ambientBase : vec3(0.0);
    vec3 diffuse  = diffuseOn  ? light.diffuse  * diff * diffuseBase : vec3(0.0);
    vec3 specular = specularOn ? light.specular * spec * specularBase : vec3(0.0);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 ambientBase, vec3 diffuseBase, vec3 specularBase, float shininess)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction));
    float intensity = (theta > light.cutOff) ? 1.0 : 0.0;

    vec3 ambient  = ambientOn  ? light.ambient  * ambientBase : vec3(0.0);
    vec3 diffuse  = diffuseOn  ? light.diffuse  * diff * diffuseBase : vec3(0.0);
    vec3 specular = specularOn ? light.specular * spec * specularBase : vec3(0.0);
    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}


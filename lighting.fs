#version 330 core
out vec4 FragColor;

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

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

#define NR_POINT_LIGHTS 36
#define NR_SPOT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform sampler2D diffuseMap;
uniform Material material;
// 0 = texture-only albedo; 1 = pure material color
uniform float materialTexBlend;

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
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 texColor = texture(diffuseMap, TexCoord).rgb;
    float blend = clamp(materialTexBlend, 0.0, 1.0);
    vec3 ambientBase = mix(texColor, material.ambient, blend);
    vec3 diffuseBase = mix(texColor, material.diffuse, blend);
    vec3 specularBase = mix(texColor, material.specular, blend);

    vec3 result = vec3(0.0);

    // Phase 1: directional light
    if (dirLightOn)
        result += CalcDirLight(dirLight, norm, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);

    // Phase 2: point lights
    if (pointLightsOn) {
        for (int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);
    }

    // Phase 3: spot lights
    if (spotLightsOn) {
        for (int i = 0; i < NR_SPOT_LIGHTS; i++)
            result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, ambientBase, diffuseBase, specularBase, material.shininess);
    }

    FragColor = vec4(result, 1.0);
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

    // Single cut-off angle (hard edge)
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

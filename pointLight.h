#ifndef pointLight_h
#define pointLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include <string>

class PointLight {
public:
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float k_c;
    float k_l;
    float k_q;
    int lightNumber;

    PointLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic, int num) {
        position = pos;
        ambient = amb;
        diffuse = diff;
        specular = spec;
        k_c = constant;
        k_l = linear;
        k_q = quadratic;
        lightNumber = num;
    }

    void setUpPointLight(Shader& lightingShader) {
        lightingShader.use();
        std::string base = "pointLights[" + std::to_string(lightNumber) + "].";
        lightingShader.setVec3(base + "position", position);
        lightingShader.setVec3(base + "ambient", ambient);
        lightingShader.setVec3(base + "diffuse", diffuse);
        lightingShader.setVec3(base + "specular", specular);
        lightingShader.setFloat(base + "constant", k_c);
        lightingShader.setFloat(base + "linear", k_l);
        lightingShader.setFloat(base + "quadratic", k_q);
    }
};

#endif /* pointLight_h */
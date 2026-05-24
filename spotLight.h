#ifndef spotLight_h
#define spotLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include <string>

class SpotLight {
public:
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float k_c;
    float k_l;
    float k_q;
    int lightNumber;

    SpotLight(glm::vec3 pos, glm::vec3 dir, float cutAngle, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic, int num) {
        position = pos;
        direction = dir;
        cutOff = cutAngle;
        ambient = amb;
        diffuse = diff;
        specular = spec;
        k_c = constant;
        k_l = linear;
        k_q = quadratic;
        lightNumber = num;
    }

    void setUpSpotLight(Shader& lightingShader) {
        lightingShader.use();
        std::string base = "spotLights[" + std::to_string(lightNumber) + "].";
        lightingShader.setVec3(base + "position", position);
        lightingShader.setVec3(base + "direction", direction);
        lightingShader.setFloat(base + "cutOff", cutOff);
        lightingShader.setVec3(base + "ambient", ambient);
        lightingShader.setVec3(base + "diffuse", diffuse);
        lightingShader.setVec3(base + "specular", specular);
        lightingShader.setFloat(base + "constant", k_c);
        lightingShader.setFloat(base + "linear", k_l);
        lightingShader.setFloat(base + "quadratic", k_q);
    }
};

#endif /* spotLight_h */
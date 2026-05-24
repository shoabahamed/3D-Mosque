#ifndef dirLight_h
#define dirLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class DirLight {
public:
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    DirLight(glm::vec3 dir, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec) {
        direction = dir;
        ambient = amb;
        diffuse = diff;
        specular = spec;
    }

    void setUpDirLight(Shader& lightingShader) {
        lightingShader.use();
        lightingShader.setVec3("dirLight.direction", direction);
        lightingShader.setVec3("dirLight.ambient", ambient);
        lightingShader.setVec3("dirLight.diffuse", diffuse);
        lightingShader.setVec3("dirLight.specular", specular);
    }
};

#endif /* dirLight_h */
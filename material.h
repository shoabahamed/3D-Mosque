#ifndef material_h
#define material_h

#include <glm/glm.hpp>

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(
        const glm::vec3& ambient = glm::vec3(1.0f),
        const glm::vec3& diffuse = glm::vec3(1.0f),
        const glm::vec3& specular = glm::vec3(1.0f),
        float shininess = 32.0f)
        : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}

    static Material FromColor(const glm::vec3& color, float shininess = 32.0f) {
        return Material(color, color, color, shininess);
    }
};

#endif /* material_h */

#ifndef cone_h
#define cone_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "shader.h"
#include "material.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Cone {
public:
    Cone(int sectors = 36) {
        setUpCone(sectors);
    }

    ~Cone() {
        glDeleteVertexArrays(1, &coneVAO);
        glDeleteBuffers(1, &coneVBO);
        glDeleteBuffers(1, &coneEBO);
    }

    void setDefaultMaterial(const Material& material) {
        defaultMaterial = material;
    }

    const Material& getDefaultMaterial() const {
        return defaultMaterial;
    }

    void draw(Shader& shader, glm::mat4 model, const Material& material) {
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.diffuse", material.diffuse);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);
        glBindVertexArray(coneVAO);
        glDrawElements(GL_TRIANGLES, coneIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void draw(Shader& shader, glm::mat4 model, glm::vec3 color) {
        draw(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

private:
    unsigned int coneVAO;
    unsigned int coneVBO;
    unsigned int coneEBO;
    unsigned int coneIndexCount;
    Material defaultMaterial;

    void setUpCone(int sectors) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        const float radius = 1.0f;
        const float halfHeight = 1.0f;

        // === SIDE ===
        for (int i = 0; i <= sectors; ++i) {
            float t = static_cast<float>(i) / sectors;
            float angle = 2.0f * static_cast<float>(M_PI) * t;
            float x = radius * std::cos(angle);
            float z = radius * std::sin(angle);

            glm::vec3 n = glm::normalize(glm::vec3(x, 0.5f, z));

            // base ring vertex
            vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);
            vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
            vertices.push_back(t); vertices.push_back(0.0f);

            // duplicated tip vertex for smooth side shading seam
            vertices.push_back(0.0f); vertices.push_back(halfHeight); vertices.push_back(0.0f);
            vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
            vertices.push_back(t); vertices.push_back(1.0f);
        }

        for (int i = 0; i < sectors; ++i) {
            unsigned int b0 = 2 * i;
            unsigned int t0 = 2 * i + 1;
            unsigned int b1 = 2 * (i + 1);
            unsigned int t1 = 2 * (i + 1) + 1;

            indices.push_back(b0);
            indices.push_back(b1);
            indices.push_back(t1);

            indices.push_back(b0);
            indices.push_back(t1);
            indices.push_back(t0);
        }

        // === BASE CAP ===
        unsigned int baseStart = static_cast<unsigned int>(vertices.size() / 8);

        // center
        vertices.push_back(0.0f); vertices.push_back(-halfHeight); vertices.push_back(0.0f);
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(0.5f); vertices.push_back(0.5f);

        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / sectors;
            float x = radius * std::cos(angle);
            float z = radius * std::sin(angle);

            vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);
            vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
            vertices.push_back(0.5f + 0.5f * std::cos(angle));
            vertices.push_back(0.5f + 0.5f * std::sin(angle));
        }

        for (int i = 0; i < sectors; ++i) {
            unsigned int next = (i + 1) % sectors;
            indices.push_back(baseStart);
            indices.push_back(baseStart + 1 + next);
            indices.push_back(baseStart + 1 + i);
        }

        coneIndexCount = static_cast<unsigned int>(indices.size());

        glGenVertexArrays(1, &coneVAO);
        glGenBuffers(1, &coneVBO);
        glGenBuffers(1, &coneEBO);

        glBindVertexArray(coneVAO);
        glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
};

#endif /* cone_h */

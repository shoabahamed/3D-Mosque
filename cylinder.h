#ifndef cylinder_h
#define cylinder_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "shader.h"
#include "material.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Cylinder {
public:
    Cylinder(int sectors = 36) {
        setUpCylinder(sectors);
    }

    ~Cylinder() {
        glDeleteVertexArrays(1, &cylinderVAO);
        glDeleteBuffers(1, &cylinderVBO);
        glDeleteBuffers(1, &cylinderEBO);
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
        glBindVertexArray(cylinderVAO);
        glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void draw(Shader& shader, glm::mat4 model, glm::vec3 color) {
        draw(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

private:
    unsigned int cylinderVAO;
    unsigned int cylinderVBO;
    unsigned int cylinderEBO;
    unsigned int cylinderIndexCount;
    Material defaultMaterial;

    void setUpCylinder(int sectors) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float halfHeight = 1.0f;
        float radius = 1.0f;

        // === BOTTOM CAP ===
        vertices.push_back(0.0f); vertices.push_back(-halfHeight); vertices.push_back(0.0f);
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(0.5f); vertices.push_back(0.5f);
        unsigned int bottomCenterIndex = 0;

        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * M_PI * i / sectors;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);
            vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
            vertices.push_back(0.5f + 0.5f * cos(angle));
            vertices.push_back(0.5f + 0.5f * sin(angle));
        }
        for (int i = 0; i < sectors; ++i) {
            unsigned int next = (i + 1) % sectors;
            indices.push_back(bottomCenterIndex);
            indices.push_back(1 + i);
            indices.push_back(1 + next);
        }

        // === TOP CAP ===
        unsigned int topCapStart = 1 + sectors;
        vertices.push_back(0.0f); vertices.push_back(halfHeight); vertices.push_back(0.0f);
        vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
        vertices.push_back(0.5f); vertices.push_back(0.5f);
        unsigned int topCenterIndex = topCapStart;

        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * M_PI * i / sectors;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(x); vertices.push_back(halfHeight); vertices.push_back(z);
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
            vertices.push_back(0.5f + 0.5f * cos(angle));
            vertices.push_back(0.5f + 0.5f * sin(angle));
        }
        for (int i = 0; i < sectors; ++i) {
            unsigned int next = (i + 1) % sectors;
            indices.push_back(topCenterIndex);
            indices.push_back(topCapStart + 1 + next);
            indices.push_back(topCapStart + 1 + i);
        }

        // === SIDE ===
        unsigned int sideStart = topCapStart + 1 + sectors;
        for (int i = 0; i < sectors; ++i) {
            float angle = 2.0f * M_PI * i / sectors;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            float nx = cos(angle);
            float nz = sin(angle);
            float u = static_cast<float>(i) / sectors;

            vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);
            vertices.push_back(nx); vertices.push_back(0.0f); vertices.push_back(nz);
            vertices.push_back(u); vertices.push_back(0.0f);

            vertices.push_back(x); vertices.push_back(halfHeight); vertices.push_back(z);
            vertices.push_back(nx); vertices.push_back(0.0f); vertices.push_back(nz);
            vertices.push_back(u); vertices.push_back(1.0f);
        }

        for (int i = 0; i < sectors; ++i) {
            unsigned int next = (i + 1) % sectors;
            unsigned int b0 = sideStart + 2 * i;
            unsigned int t0 = sideStart + 2 * i + 1;
            unsigned int b1 = sideStart + 2 * next;
            unsigned int t1 = sideStart + 2 * next + 1;

            indices.push_back(b0);
            indices.push_back(b1);
            indices.push_back(t1);
            indices.push_back(b0);
            indices.push_back(t1);
            indices.push_back(t0);
        }

        cylinderIndexCount = static_cast<unsigned int>(indices.size());

        glGenVertexArrays(1, &cylinderVAO);
        glGenBuffers(1, &cylinderVBO);
        glGenBuffers(1, &cylinderEBO);

        glBindVertexArray(cylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO);
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

#endif /* cylinder_h */
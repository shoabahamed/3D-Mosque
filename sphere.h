#ifndef sphere_h
#define sphere_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "shader.h"
#include "material.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Sphere {
public:
    Sphere(int stacks = 18, int slices = 36) {
        this->stacks = stacks;
        this->slices = slices;
        setUpSphere();
        setUpDomeHemisphereRadial();
    }

    ~Sphere() {
        glDeleteVertexArrays(1, &sphereVAO);
        glDeleteBuffers(1, &sphereVBO);
        glDeleteBuffers(1, &sphereEBO);

        glDeleteVertexArrays(1, &domeHemisphereVAO);
        glDeleteBuffers(1, &domeHemisphereVBO);
        glDeleteBuffers(1, &domeHemisphereEBO);
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
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void draw(Shader& shader, glm::mat4 model, glm::vec3 color) {
        draw(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void drawHemisphere(Shader& shader, glm::mat4 model, const Material& material) {
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.diffuse", material.diffuse);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);
        glBindVertexArray(sphereVAO);
        int halfIndices = (stacks / 2) * slices * 6;
        glDrawElements(GL_TRIANGLES, halfIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Helper: draw a hemisphere (top half of sphere) using the sphere VAO
    void drawHemisphere(Shader& shader, glm::mat4 model, glm::vec3 color) {
        drawHemisphere(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void drawDomeHemisphere(Shader& shader, glm::mat4 model, const Material& material) {
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.diffuse", material.diffuse);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);
        glBindVertexArray(domeHemisphereVAO);
        glDrawElements(GL_TRIANGLES, domeHemisphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void drawDomeHemisphere(Shader& shader, glm::mat4 model, glm::vec3 color) {
        drawDomeHemisphere(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

private:
    unsigned int sphereVAO = 0;
    unsigned int sphereVBO = 0;
    unsigned int sphereEBO = 0;
    unsigned int sphereIndexCount = 0;

    unsigned int domeHemisphereVAO = 0;
    unsigned int domeHemisphereVBO = 0;
    unsigned int domeHemisphereEBO = 0;
    unsigned int domeHemisphereIndexCount = 0;

    int stacks;
    int slices;
    Material defaultMaterial;

    void setUpSphere() {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float radius = 1.0f;

        for (int i = 0; i <= stacks; ++i) {
            float phi = M_PI * i / stacks;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            float v = 1.0f - static_cast<float>(i) / stacks;

            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * M_PI * j / slices;
                float sinTheta = sin(theta);
                float cosTheta = cos(theta);
                float u = static_cast<float>(j) / slices;

                float x = radius * sinPhi * cosTheta;
                float y = radius * cosPhi;
                float z = radius * sinPhi * sinTheta;

                // position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                // normal (same as position for unit sphere)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                // texture coordinates
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                unsigned int first = i * (slices + 1) + j;
                unsigned int second = first + 1;
                unsigned int third = (i + 1) * (slices + 1) + j;
                unsigned int fourth = third + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(third);
                indices.push_back(second);
                indices.push_back(fourth);
                indices.push_back(third);
            }
        }

        sphereIndexCount = static_cast<unsigned int>(indices.size());

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    // OLD DOME UV MAPPING (lat-long style on hemisphere) - kept for comparison
    // void setUpDomeHemisphere() {
    //     std::vector<float> vertices;
    //     std::vector<unsigned int> indices;
    //
    //     const float radius = 1.0f;
    //     int hemiStacks = stacks / 2;
    //     if (hemiStacks < 1)
    //         hemiStacks = 1;
    //
    //     for (int i = 0; i <= hemiStacks; ++i) {
    //         float phi = (M_PI * 0.5f) * static_cast<float>(i) / static_cast<float>(hemiStacks);
    //         float sinPhi = sin(phi);
    //         float cosPhi = cos(phi);
    //
    //         // full vertical usage for hemisphere texture: top->bottom maps 1..0
    //         float v = 1.0f - static_cast<float>(i) / static_cast<float>(hemiStacks);
    //
    //         for (int j = 0; j <= slices; ++j) {
    //             float theta = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
    //             float sinTheta = sin(theta);
    //             float cosTheta = cos(theta);
    //             float u = static_cast<float>(j) / static_cast<float>(slices);
    //
    //             float x = radius * sinPhi * cosTheta;
    //             float y = radius * cosPhi;
    //             float z = radius * sinPhi * sinTheta;
    //
    //             vertices.push_back(x);
    //             vertices.push_back(y);
    //             vertices.push_back(z);
    //             vertices.push_back(x);
    //             vertices.push_back(y);
    //             vertices.push_back(z);
    //             vertices.push_back(u);
    //             vertices.push_back(v);
    //         }
    //     }
    //
    //     for (int i = 0; i < hemiStacks; ++i) {
    //         for (int j = 0; j < slices; ++j) {
    //             unsigned int first = i * (slices + 1) + j;
    //             unsigned int second = first + 1;
    //             unsigned int third = (i + 1) * (slices + 1) + j;
    //             unsigned int fourth = third + 1;
    //
    //             indices.push_back(first);
    //             indices.push_back(second);
    //             indices.push_back(third);
    //             indices.push_back(second);
    //             indices.push_back(fourth);
    //             indices.push_back(third);
    //         }
    //     }
    //
    //     domeHemisphereIndexCount = static_cast<unsigned int>(indices.size());
    //
    //     glGenVertexArrays(1, &domeHemisphereVAO);
    //     glGenBuffers(1, &domeHemisphereVBO);
    //     glGenBuffers(1, &domeHemisphereEBO);
    //
    //     glBindVertexArray(domeHemisphereVAO);
    //     glBindBuffer(GL_ARRAY_BUFFER, domeHemisphereVBO);
    //     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, domeHemisphereEBO);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    //
    //     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    //     glEnableVertexAttribArray(0);
    //     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //     glEnableVertexAttribArray(1);
    //     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //     glEnableVertexAttribArray(2);
    //     glBindVertexArray(0);
    // }

    void setUpDomeHemisphereRadial() {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        const float radius = 1.0f;
        int hemiStacks = stacks / 2;
        if (hemiStacks < 1)
            hemiStacks = 1;

        for (int i = 0; i <= hemiStacks; ++i) {
            float phi = (M_PI * 0.5f) * static_cast<float>(i) / static_cast<float>(hemiStacks);
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float radial = sinPhi;

            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
                float sinTheta = sin(theta);
                float cosTheta = cos(theta);

                float x = radius * sinPhi * cosTheta;
                float y = radius * cosPhi;
                float z = radius * sinPhi * sinTheta;

                // Circle-to-square radial remap:
                // 1) Build direction on unit circle
                float dx = cosTheta;
                float dy = sinTheta;

                // 2) Scale so ray reaches square boundary instead of circle boundary
                float maxAbs = std::max(std::fabs(dx), std::fabs(dy));
                if (maxAbs < 1e-6f)
                    maxAbs = 1.0f;
                float sx = dx / maxAbs;
                float sy = dy / maxAbs;

                // 3) Interpolate from center (top) to square boundary with radial distance
                float u = 0.5f + 0.5f * radial * sx;
                float v = 0.5f + 0.5f * radial * sy;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        for (int i = 0; i < hemiStacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                unsigned int first = i * (slices + 1) + j;
                unsigned int second = first + 1;
                unsigned int third = (i + 1) * (slices + 1) + j;
                unsigned int fourth = third + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(third);
                indices.push_back(second);
                indices.push_back(fourth);
                indices.push_back(third);
            }
        }

        domeHemisphereIndexCount = static_cast<unsigned int>(indices.size());

        glGenVertexArrays(1, &domeHemisphereVAO);
        glGenBuffers(1, &domeHemisphereVBO);
        glGenBuffers(1, &domeHemisphereEBO);

        glBindVertexArray(domeHemisphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, domeHemisphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, domeHemisphereEBO);
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

#endif /* sphere_h */
#ifndef cube_h
#define cube_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "material.h"

class Cube
{
public:
    Cube()
    {
        setUpCubeVertexDataAndConfigureVertexAttribute();
    }

    ~Cube()
    {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteBuffers(1, &cubeEBO);
    }

    void setDefaultMaterial(const Material &material)
    {
        defaultMaterial = material;
    }

    const Material &getDefaultMaterial() const
    {
        return defaultMaterial;
    }

    void draw(Shader &shader, glm::mat4 model, const Material &material)
    {
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", material.ambient);
        shader.setVec3("material.diffuse", material.diffuse);
        shader.setVec3("material.specular", material.specular);
        shader.setFloat("material.shininess", material.shininess);

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void draw(Shader &shader, glm::mat4 model, glm::vec3 color)
    {
        draw(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

private:
    unsigned int cubeVAO;
    unsigned int cubeVBO;
    unsigned int cubeEBO;
    Material defaultMaterial;

    void setUpCubeVertexDataAndConfigureVertexAttribute()
    {
        float cubeVertices[] = {
            // Front face
            -0.5f,
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.5f,
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            0.5f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -0.5f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            // Back face
            -0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            1.0f,
            0.0f,
            0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.5f,
            0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            0.0f,
            1.0f,
            -0.5f,
            0.5f,
            -0.5f,
            0.0f,
            0.0f,
            -1.0f,
            1.0f,
            1.0f,
            // Left face
            -0.5f,
            -0.5f,
            -0.5f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            -0.5f,
            -0.5f,
            0.5f,
            -1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            -0.5f,
            0.5f,
            0.5f,
            -1.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            -0.5f,
            0.5f,
            -0.5f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            // Right face
            0.5f,
            -0.5f,
            -0.5f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.5f,
            -0.5f,
            0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.5f,
            0.5f,
            0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            0.5f,
            0.5f,
            -0.5f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            // Top face
            -0.5f,
            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            -0.5f,
            0.5f,
            0.5f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            0.5f,
            0.5f,
            0.5f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.5f,
            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f,
            // Bottom face
            -0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            -1.0f,
            0.0f,
            0.0f,
            1.0f,
            -0.5f,
            -0.5f,
            0.5f,
            0.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.5f,
            -0.5f,
            0.5f,
            0.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
            0.5f,
            -0.5f,
            -0.5f,
            0.0f,
            -1.0f,
            0.0f,
            1.0f,
            1.0f,
        };
        unsigned int cubeIndices[] = {
            0,
            1,
            2,
            2,
            3,
            0,
            6,
            5,
            4,
            4,
            7,
            6,
            8,
            9,
            10,
            10,
            11,
            8,
            14,
            13,
            12,
            12,
            15,
            14,
            16,
            17,
            18,
            18,
            19,
            16,
            22,
            21,
            20,
            20,
            23,
            22,
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
};

#endif /* cube_h */
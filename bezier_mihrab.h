#ifndef bezier_mihrab_h
#define bezier_mihrab_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "shader.h"
#include "material.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// BezierMihrab
// ============================================================================
// Generates a curved mihrab (prayer niche) whose plan-view profile is a
// cubic Bézier curve in the XZ plane.
//
// By using a Cubic Bézier with four control points, pulling the depth
// backward doesn't squeeze the center. Instead it creates a broad U-shape
// ensuring things like the chair and fan stay safely within the boundaries.
//
// Generates:
//   1. Curved WALL  – the Bézier profile extruded vertically. (normals inward)
//   2. Curved ROOF  – the top cap polygon at topY. (normals down)
//   3. Curved FLOOR – the inner floor polygon at floorY. (normals up)
//   4. BASE VOLUME  – the solid base from 0.0 to floorY. (normals outward)
// ============================================================================

class BezierMihrab
{
public:
    BezierMihrab(float leftX, float rightX, float backWallZ, float depth,
                 float bottomY, float floorY, float topY, int segments = 48)
    {
        build(leftX, rightX, backWallZ, depth, bottomY, floorY, topY, segments);
    }

    ~BezierMihrab()
    {
        glDeleteVertexArrays(1, &wallVAO);
        glDeleteBuffers(1, &wallVBO);
        glDeleteBuffers(1, &wallEBO);
        glDeleteVertexArrays(1, &roofVAO);
        glDeleteBuffers(1, &roofVBO);
        glDeleteBuffers(1, &roofEBO);
        glDeleteVertexArrays(1, &floorVAO);
        glDeleteBuffers(1, &floorVBO);
        glDeleteBuffers(1, &floorEBO);
        glDeleteVertexArrays(1, &baseVAO);
        glDeleteBuffers(1, &baseVBO);
        glDeleteBuffers(1, &baseEBO);
    }

    void drawWall(Shader &shader, glm::mat4 model, const Material &mat)
    {
        setMaterialAndDraw(shader, model, mat, wallVAO, wallIndexCount);
    }
    void drawWall(Shader &shader, glm::mat4 model, glm::vec3 color)
    {
        drawWall(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void drawRoof(Shader &shader, glm::mat4 model, const Material &mat)
    {
        setMaterialAndDraw(shader, model, mat, roofVAO, roofIndexCount);
    }
    void drawRoof(Shader &shader, glm::mat4 model, glm::vec3 color)
    {
        drawRoof(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void drawFloor(Shader &shader, glm::mat4 model, const Material &mat)
    {
        setMaterialAndDraw(shader, model, mat, floorVAO, floorIndexCount);
    }
    void drawFloor(Shader &shader, glm::mat4 model, glm::vec3 color)
    {
        drawFloor(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void drawBaseVolume(Shader &shader, glm::mat4 model, const Material &mat)
    {
        setMaterialAndDraw(shader, model, mat, baseVAO, baseIndexCount);
    }
    void drawBaseVolume(Shader &shader, glm::mat4 model, glm::vec3 color)
    {
        drawBaseVolume(shader, model, Material::FromColor(color, defaultMaterial.shininess));
    }

    void setDefaultMaterial(const Material &m) { defaultMaterial = m; }

private:
    unsigned int wallVAO = 0, wallVBO = 0, wallEBO = 0, wallIndexCount = 0;
    unsigned int roofVAO = 0, roofVBO = 0, roofEBO = 0, roofIndexCount = 0;
    unsigned int floorVAO = 0, floorVBO = 0, floorEBO = 0, floorIndexCount = 0;
    unsigned int baseVAO = 0, baseVBO = 0, baseEBO = 0, baseIndexCount = 0;

    Material defaultMaterial;

    void setMaterialAndDraw(Shader &shader, glm::mat4 model, const Material &mat,
                            unsigned int vao, unsigned int idxCount)
    {
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", mat.ambient);
        shader.setVec3("material.diffuse", mat.diffuse);
        shader.setVec3("material.specular", mat.specular);
        shader.setFloat("material.shininess", mat.shininess);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    static void uploadMesh(const std::vector<float> &verts,
                           const std::vector<unsigned int> &idxs,
                           unsigned int &vao, unsigned int &vbo, unsigned int &ebo,
                           unsigned int &idxCount)
    {
        idxCount = (unsigned int)idxs.size();
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxs.size() * sizeof(unsigned int), idxs.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    static void pushVert(std::vector<float> &v,
                         float px, float py, float pz,
                         float nx, float ny, float nz,
                         float u, float uv_v)
    {
        v.insert(v.end(), {px, py, pz, nx, ny, nz, u, uv_v});
    }

    // Evaluate cubic Bézier at parameter t ∈ [0,1].
    static glm::vec2 bezierCubic(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t)
    {
        float u = 1.0f - t;
        float u2 = u * u;
        float u3 = u2 * u;
        float t2 = t * t;
        float t3 = t2 * t;

        return u3 * p0 + 3.0f * u2 * t * p1 + 3.0f * u * t2 * p2 + t3 * p3;
    }

    // Tangent of cubic Bézier (derivative).
    static glm::vec2 bezierCubicTangent(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t)
    {
        float u = 1.0f - t;
        float u2 = u * u;
        float t2 = t * t;

        return 3.0f * u2 * (p1 - p0) + 6.0f * u * t * (p2 - p1) + 3.0f * t2 * (p3 - p2);
    }

    void build(float leftX, float rightX, float backWallZ, float depth,
               float bottomY, float floorY, float topY, int segments)
    {
        // Cubic Bézier control points in XZ plane.
        // P0 = left endpoint
        // P1 = pushes straight back from the left (U-shape start)
        // P2 = pushes straight back from the right
        // P3 = right endpoint
        glm::vec2 P0(leftX, backWallZ);
        glm::vec2 P1(leftX, backWallZ - depth);
        glm::vec2 P2(rightX, backWallZ - depth);
        glm::vec2 P3(rightX, backWallZ);

        // 1. CURVED WALL (inward facing)
        {
            std::vector<float> verts;
            std::vector<unsigned int> idxs;
            for (int i = 0; i <= segments; ++i)
            {
                float t = (float)i / (float)segments;
                glm::vec2 pt = bezierCubic(P0, P1, P2, P3, t);
                glm::vec2 tg = bezierCubicTangent(P0, P1, P2, P3, t);

                // inward normal
                glm::vec2 n2d(tg.y, -tg.x);
                float len = glm::length(n2d);
                if (len > 1e-6f)
                    n2d /= -len;

                pushVert(verts, pt.x, floorY, pt.y, n2d.x, 0.0f, n2d.y, t, 0.0f);
                pushVert(verts, pt.x, topY, pt.y, n2d.x, 0.0f, n2d.y, t, 1.0f);
            }
            for (int i = 0; i < segments; ++i)
            {
                unsigned int b0 = i * 2, t0 = i * 2 + 1, b1 = (i + 1) * 2, t1 = (i + 1) * 2 + 1;
                idxs.push_back(b0);
                idxs.push_back(t0);
                idxs.push_back(t1);
                idxs.push_back(b0);
                idxs.push_back(t1);
                idxs.push_back(b1);
            }
            uploadMesh(verts, idxs, wallVAO, wallVBO, wallEBO, wallIndexCount);
        }

        auto generateCap = [&](std::vector<float> &verts, std::vector<unsigned int> &idxs,
                               float y, float ny, bool faceUp)
        {
            float centerX = (leftX + rightX) * 0.5f;
            pushVert(verts, centerX, y, backWallZ, 0.0f, ny, 0.0f, 0.5f, 0.0f);
            for (int i = 0; i <= segments; ++i)
            {
                float t = (float)i / (float)segments;
                glm::vec2 pt = bezierCubic(P0, P1, P2, P3, t);
                float u = (pt.x - leftX) / (rightX - leftX);
                float v = (depth > 0.01f) ? ((backWallZ - pt.y) / depth) : 1.0f;
                pushVert(verts, pt.x, y, pt.y, 0.0f, ny, 0.0f, u, v);
            }
            for (int i = 0; i < segments; ++i)
            {
                idxs.push_back(0);
                if (faceUp)
                {
                    idxs.push_back(1 + i);
                    idxs.push_back(1 + i + 1);
                }
                else
                {
                    idxs.push_back(1 + i + 1);
                    idxs.push_back(1 + i);
                }
            }
        };

        // 2. ROOF CAP (facing DOWN)
        {
            std::vector<float> verts;
            std::vector<unsigned int> idxs;
            generateCap(verts, idxs, topY, -1.0f, false);
            uploadMesh(verts, idxs, roofVAO, roofVBO, roofEBO, roofIndexCount);
        }

        // 3. FLOOR CAP (facing UP)
        {
            std::vector<float> verts;
            std::vector<unsigned int> idxs;
            generateCap(verts, idxs, floorY, 1.0f, true);
            uploadMesh(verts, idxs, floorVAO, floorVBO, floorEBO, floorIndexCount);
        }

        // 4. BASE VOLUME (extruded curve from y=bottomY to y=floorY facing OUTWARD)
        {
            std::vector<float> verts;
            std::vector<unsigned int> idxs;

            // Generate bottom cap (facing DOWN)
            unsigned int bottomStart = 0;
            generateCap(verts, idxs, bottomY, -1.0f, false);

            // Generate outward side surfaces
            unsigned int sideStart = (unsigned int)(verts.size() / 8);
            for (int i = 0; i <= segments; ++i)
            {
                float t = (float)i / (float)segments;
                glm::vec2 pt = bezierCubic(P0, P1, P2, P3, t);
                glm::vec2 tg = bezierCubicTangent(P0, P1, P2, P3, t);

                // outward normal
                glm::vec2 n2d(tg.y, -tg.x);
                float len = glm::length(n2d);
                if (len > 1e-6f)
                    n2d /= len; // Point OUTWARD

                pushVert(verts, pt.x, bottomY, pt.y, n2d.x, 0.0f, n2d.y, t, 0.0f);
                pushVert(verts, pt.x, floorY, pt.y, n2d.x, 0.0f, n2d.y, t, 1.0f);
            }
            for (int i = 0; i < segments; ++i)
            {
                unsigned int b0 = sideStart + i * 2, t0 = sideStart + i * 2 + 1;
                unsigned int b1 = sideStart + (i + 1) * 2, t1 = sideStart + (i + 1) * 2 + 1;
                // outward winding
                idxs.push_back(b0);
                idxs.push_back(b1);
                idxs.push_back(t0);
                idxs.push_back(t0);
                idxs.push_back(b1);
                idxs.push_back(t1);
            }

            uploadMesh(verts, idxs, baseVAO, baseVBO, baseEBO, baseIndexCount);
        }
    }
};

#endif /* bezier_mihrab_h */

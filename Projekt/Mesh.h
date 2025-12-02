#pragma once
#include <glad/glad.h>
#include <vector>
#include <cmath>

struct SphereMesh {
    unsigned int VAO;
    unsigned int indexCount;
};

// --- FUNKCJA GENERUJ¥CA SFERÊ (DLA PLANET) ---
inline SphereMesh generateSphere(unsigned int X_SEGMENTS, unsigned int Y_SEGMENTS) {
    SphereMesh mesh;
    std::vector<float> data;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359f;

    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;

            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            // 1. Pozycja (x, y, z)
            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);

            // 2. Normalna (to samo co pozycja dla sfery jednostkowej)
            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);

            // 3. UV (Tekstura)
            data.push_back(1.0f - xSegment);
            data.push_back(ySegment);
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        if (!oddRow) {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else {
            for (int x = X_SEGMENTS; x >= 0; --x) {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    // Generowanie indeksów dla GL_TRIANGLES (bardziej uniwersalne ni¿ Triangle Strip w tym przypadku)
    indices.clear();
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
        }
    }

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Stride: 3 pos + 3 norm + 2 uv = 8 floats
    int stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    mesh.indexCount = (unsigned int)indices.size();
    return mesh;
}

// --- NOWA FUNKCJA: GENERUJ¥CA PIERŒCIEÑ (DLA SATURNA) ---
inline SphereMesh generateRing(float innerRadius, float outerRadius, int segments) {
    SphereMesh mesh;
    std::vector<float> data;
    std::vector<unsigned int> indices;
    const float PI = 3.14159265359f;

    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / (float)segments * 2.0f * PI;
        float x = cos(angle);
        float z = sin(angle);

        // -- Wierzcho³ek Wewnêtrzny --
        data.push_back(x * innerRadius); data.push_back(0.0f); data.push_back(z * innerRadius); // Poz
        data.push_back(0.0f); data.push_back(1.0f); data.push_back(0.0f); // Norm
        data.push_back((float)i / segments); data.push_back(0.0f); // UV (V=0)

        // -- Wierzcho³ek Zewnêtrzny --
        data.push_back(x * outerRadius); data.push_back(0.0f); data.push_back(z * outerRadius); // Poz
        data.push_back(0.0f); data.push_back(1.0f); data.push_back(0.0f); // Norm
        data.push_back((float)i / segments); data.push_back(1.0f); // UV (V=1)
    }

    for (int i = 0; i < segments; ++i) {
        unsigned int innerCurrent = 2 * i;
        unsigned int outerCurrent = 2 * i + 1;
        unsigned int innerNext = 2 * (i + 1);
        unsigned int outerNext = 2 * (i + 1) + 1;

        indices.push_back(innerCurrent);
        indices.push_back(outerCurrent);
        indices.push_back(innerNext);

        indices.push_back(outerCurrent);
        indices.push_back(outerNext);
        indices.push_back(innerNext);
    }

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    mesh.indexCount = (unsigned int)indices.size();
    return mesh;
}
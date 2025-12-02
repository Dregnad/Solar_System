#pragma once

// Wa¿ne: U¿ywamy glad/glad.h tak jak w main.cpp, aby unikn¹æ b³êdów kompilacji
#include <glad/glad.h> 
#include <vector>
#include <cmath>

struct SphereMesh {
    unsigned int VAO;
    unsigned int indexCount;
};

// Funkcja inline pozwala na definicjê w pliku .h bez b³êdów linkera
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

            // Pozycja
            data.push_back(xPos); data.push_back(yPos); data.push_back(zPos);
            // Normalna
            data.push_back(xPos); data.push_back(yPos); data.push_back(zPos);
            // UV (Lustrzana poprawka z Twojego main.cpp)
            data.push_back(1.0f - xSegment); data.push_back(ySegment);
        }
    }

    // Generowanie indeksów dla GL_TRIANGLES
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
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    mesh.indexCount = (unsigned int)indices.size();
    return mesh;
}
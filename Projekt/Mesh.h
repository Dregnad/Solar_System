#pragma once
#include <glad/glad.h>
#include <vector>
#include <cmath>

struct SphereMesh {
    unsigned int VAO;
    unsigned int VBO; // Dodano, aby trzymaæ uchwyt do bufora
    unsigned int EBO; // Dodano, aby trzymaæ uchwyt do bufora indeksów
    unsigned int indexCount;
};

// --- FUNKCJA GENERUJ¥CA SFERÊ (POPRAWIONA - SZCZELNA DLA LOW POLY) ---
inline SphereMesh generateSphere(unsigned int sectorCount, unsigned int stackCount) {
    SphereMesh mesh;
    std::vector<float> data;
    std::vector<unsigned int> indices;

    float x, y, z, xy;                              // pozycje
    float nx, ny, nz, lengthInv = 1.0f / 1.0f;      // normalne (promieñ = 1.0)
    float s, t;                                     // UV

    float sectorStep = 2 * 3.14159265359f / sectorCount;
    float stackStep = 3.14159265359f / stackCount;
    float sectorAngle, stackAngle;

    // 1. GENEROWANIE WIERZCHO£KÓW
    // U¿ywamy <= aby zdublowaæ wierzcho³ki na szwie (dla poprawnego teksturowania)
    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = 3.14159265359f / 2 - i * stackStep;        // od pi/2 do -pi/2
        xy = cosf(stackAngle);             // r * cos(u)
        z = sinf(stackAngle);              // r * sin(u)

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // od 0 do 2pi

            // Pozycja
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            
            // -- POSITION --
            data.push_back(x);
            data.push_back(y);
            data.push_back(z);

            // -- NORMAL --
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            data.push_back(nx);
            data.push_back(ny);
            data.push_back(nz);

            // -- TEX COORD --
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            data.push_back(s);
            data.push_back(t);
        }
    }

    // 2. GENEROWANIE INDEKSÓW (Trójk¹ty)
    int k1, k2;
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // pocz¹tek obecnego stosu
        k2 = k1 + sectorCount + 1;      // pocz¹tek nastêpnego stosu

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // Dwa trójk¹ty na sektor (oprócz biegunów gdzie jest jeden)
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    mesh.indexCount = (unsigned int)indices.size();

    // 3. OQL I BUFORY
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Stride: 3 pos + 3 norm + 2 uv = 8 floats
    long long stride = 8 * sizeof(float);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    // TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    return mesh;
}

// --- GENEROWANIE PIERŒCIENIA (SATURN) ---
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

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    mesh.indexCount = (unsigned int)indices.size();
    glBindVertexArray(0);
    
    return mesh;
}
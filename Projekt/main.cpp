#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "Camera.h"
#include "Shader.h"
#include "Body.h"

// --- ZMIENNE GLOBALNE ---
double timeMultiplier = 1.0;
double simulationSpeed = 0.0;
double currentTimeDays = 0.0;
double deltaTimeDouble = 0.0;
double lastFrameDouble = 0.0;

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 300.0f, 700.0f));
float baseCameraSpeed = 200.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorLocked = true;
bool isPaused = false;
bool renderOrbits = true;

const float UNIFIED_RADIUS_SCALE = 0.00004f;

// --- GENERATOR SFERY ---
struct SphereMesh {
    unsigned int VAO;
    unsigned int indexCount;
};

SphereMesh generateSphere(unsigned int X_SEGMENTS, unsigned int Y_SEGMENTS) {
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
            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);
            // Normalna
            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);

            // --- POPRAWKA LUSTRZANA ---
            // Odwracamy oœ U (1.0 - x), ¿eby mapa œwiata nie by³a odbiciem lustrzanym
            data.push_back(1.0f - xSegment);
            data.push_back(ySegment);
        }
    }

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

// --- FUNKCJA £ADUJ¥CA TEKSTURY ---
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    // --- POPRAWKA PIONOWA ---
    // Ustawiamy na false, bo Twoja sfera oczekuje tekstury wczytanej "od góry"
    stbi_set_flip_vertically_on_load(false);

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

// --- OBS£UGA WEJŒCIA (bez zmian) ---
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!cursorLocked) return;
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.MovementSpeed = baseCameraSpeed * 5.0f;
    else
        camera.MovementSpeed = baseCameraSpeed;

    float dt = (float)deltaTimeDouble;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, dt);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP, dt);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, dt);

    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        cursorLocked = !cursorLocked;
        glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        tabPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tabPressed = false;
}

// --- MAIN ---
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System - Skybox & Textures", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader planetShader("planet.vert", "planet.frag");
    Shader orbitShader("orbit.vert", "orbit.frag");
    SphereMesh sphere = generateSphere(64, 64);

    // --- CIA£A NIEBIESKIE ---
    CelestialBody sun("Sun", 696340, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, 25.0, 7.25, nullptr);
    sun.color = glm::vec3(1.0f, 0.9f, 0.0f);

    CelestialBody mercury("Mercury", 2440, { 0.387, 0.2056, 7.00, 48.33, 29.12, 174.79, 0.24 }, 58.6, 0.03, &sun);
    mercury.color = glm::vec3(0.6f, 0.6f, 0.6f);

    CelestialBody venus("Venus", 6052, { 0.723, 0.0067, 3.39, 76.68, 54.88, 50.11, 0.615 }, 243.0, 177.3, &sun);
    venus.color = glm::vec3(0.9f, 0.8f, 0.6f);

    CelestialBody earth("Earth", 6371, { 1.000, 0.0167, 0.000, -11.26, 102.94, 100.46, 1.0 }, 1.0, 23.5, &sun);
    earth.color = glm::vec3(0.0f, 0.4f, 0.8f);

    CelestialBody moon("Moon", 1737, { 0.00257, 0.0549, 5.145, 125.08, 318.15, 135.27, 0.0748 }, 27.3, 6.7, &earth);
    moon.color = glm::vec3(0.7f, 0.7f, 0.7f);

    CelestialBody mars("Mars", 3390, { 1.524, 0.0934, 1.85, 49.58, 286.50, 19.37, 1.88 }, 1.03, 25.2, &sun);
    mars.color = glm::vec3(0.8f, 0.3f, 0.2f);

    CelestialBody jupiter("Jupiter", 69911, { 5.204, 0.0489, 1.304, 100.46, 273.86, 20.02, 11.86 }, 0.41, 3.13, &sun);
    jupiter.color = glm::vec3(0.8f, 0.7f, 0.5f);

    CelestialBody saturn("Saturn", 58232, { 9.582, 0.0565, 2.48, 113.71, 92.43, 317.02, 29.45 }, 0.44, 26.7, &sun);
    saturn.color = glm::vec3(0.9f, 0.85f, 0.5f);

    CelestialBody uranus("Uranus", 25362, { 19.201, 0.0463, 0.77, 74.00, 170.96, 142.23, 84.02 }, 0.72, 97.8, &sun);
    uranus.color = glm::vec3(0.5f, 0.8f, 0.9f);

    CelestialBody neptune("Neptune", 24622, { 30.047, 0.0094, 1.77, 131.78, 44.97, 267.76, 164.79 }, 0.67, 28.3, &sun);
    neptune.color = glm::vec3(0.2f, 0.2f, 0.8f);

    std::vector<CelestialBody*> bodies = { &sun, &mercury, &venus, &earth, &moon, &mars, &jupiter, &saturn, &uranus, &neptune };

    // --- £ADOWANIE TEKSTUR ---
    // Upewnij siê, ¿e pliki s¹ w assets/textures/
    unsigned int sunTex = loadTexture("sun.jpg");
    unsigned int earthTex = loadTexture("earth.jpg");
    unsigned int earthNightTex = loadTexture("earth_night.jpg");
    unsigned int milkwayTex = loadTexture("milkway.jpg");

    sun.diffuseMap = sunTex;
    earth.diffuseMap = earthTex;
    earth.nightMap = earthNightTex;

    // Konfiguracja shaderów
    planetShader.use();
    planetShader.setInt("diffuseTexture", 0);
    planetShader.setInt("nightTexture", 1);

    // --- PÊTLA G£ÓWNA ---
    while (!glfwWindowShouldClose(window)) {
        double currentFrameDouble = glfwGetTime();
        deltaTimeDouble = currentFrameDouble - lastFrameDouble;
        lastFrameDouble = currentFrameDouble;
        simulationSpeed = timeMultiplier * (1.0 / 86400.0);

        processInput(window);

        if (!isPaused) {
            currentTimeDays += simulationSpeed * deltaTimeDouble;
            for (auto body : bodies) body->update(currentTimeDays);
        }

        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ==========================================
        // --- RYSOWANIE T£A (DROGA MLECZNA) ---
        // ==========================================
        planetShader.use();
        planetShader.setMat4("projection", projection);

        // Wycinamy translacjê (ruch) z kamery, zostawiamy tylko obrót
        glm::mat4 viewSkybox = glm::mat4(glm::mat3(view));
        planetShader.setMat4("view", viewSkybox);

        planetShader.setBool("isSun", true);
        planetShader.setBool("hasTexture", true);
        planetShader.setBool("hasNightTexture", false);
        planetShader.setVec3("objectColor", glm::vec3(1.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, milkwayTex);

        glm::mat4 modelSky = glm::mat4(1.0f);
        // Odwracamy sferê (-5000), ¿eby widzieæ teksturê od œrodka
        modelSky = glm::scale(modelSky, glm::vec3(-5000.0f, -5000.0f, -5000.0f));
        planetShader.setMat4("model", modelSky);

        glDepthMask(GL_FALSE); // Nie zapisuj t³a do bufora g³êbokoœci (zawsze w tle)
        glBindVertexArray(sphere.VAO);
        glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);  // Przywróæ zapis g³êbokoœci
        // ==========================================


        // --- RYSOWANIE ORBIT ---
        if (renderOrbits) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);

            for (auto body : bodies) {
                if (body->parent == nullptr) continue;
                if (body->VAO_Orbit == 0) {
                    glGenVertexArrays(1, &body->VAO_Orbit);
                    unsigned int VBO_Orbit;
                    glGenBuffers(1, &VBO_Orbit);
                    glBindVertexArray(body->VAO_Orbit);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO_Orbit);
                    glBufferData(GL_ARRAY_BUFFER, body->orbitPath.size() * sizeof(glm::vec3), &body->orbitPath[0], GL_STATIC_DRAW);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                }
                glBindVertexArray(body->VAO_Orbit);
                glm::mat4 model = glm::mat4(1.0f);
                if (body->parent) model = glm::translate(model, glm::vec3(body->parent->worldPosition));
                orbitShader.setMat4("model", model);
                glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)body->orbitPath.size());
            }
        }

        // --- RYSOWANIE PLANET ---
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", glm::vec3(0.0f));
        planetShader.setVec3("viewPos", camera.Position);

        glBindVertexArray(sphere.VAO);

        for (auto body : bodies) {
            glm::mat4 model = glm::mat4(1.0f);
            glm::vec3 drawPos = glm::vec3(body->worldPosition);
            model = glm::translate(model, drawPos);

            float r_scale = (float)body->radius * UNIFIED_RADIUS_SCALE;

            // Ustawienia
            planetShader.setVec3("objectColor", body->color);
            planetShader.setBool("isSun", (body->name == "Sun"));

            // Tekstura dzienna
            if (body->diffuseMap != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, body->diffuseMap);
                planetShader.setBool("hasTexture", true);
            }
            else {
                planetShader.setBool("hasTexture", false);
            }

            // Tekstura nocna
            if (body->nightMap != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, body->nightMap);
                planetShader.setBool("hasNightTexture", true);
            }
            else {
                planetShader.setBool("hasNightTexture", false);
            }

            model = glm::scale(model, glm::vec3(r_scale));
            model = glm::rotate(model, glm::radians((float)body->axialTilt), glm::vec3(0, 0, 1));
            model = glm::rotate(model, glm::radians((float)body->currentRotationAngle), glm::vec3(0, 1, 0));
            planetShader.setMat4("model", model);

            glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);
        }

        // --- GUI ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sterowanie");
        ImGui::Text("Dzien symulacji: %.2f", currentTimeDays);
        ImGui::Checkbox("Pauza", &isPaused);
        ImGui::Checkbox("Orbity", &renderOrbits);

        ImGui::Separator();
        ImGui::Text("Symulacja:");
        float timeMultFloat = (float)timeMultiplier;
        if (ImGui::SliderFloat("Predkosc Czasu", &timeMultFloat, 0.0f, 100000.0f, "%.0fx")) {
            timeMultiplier = (double)timeMultFloat;
        }

        // --- TU JEST TWOJA ZGUBIONA PRÊDKOŒÆ KAMERY ---
        ImGui::Separator();
        ImGui::Text("Kamera:");
        ImGui::SliderFloat("Szybkosc Latania", &baseCameraSpeed, 10.0f, 5000.0f);
        // ----------------------------------------------

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
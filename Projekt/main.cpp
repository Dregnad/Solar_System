#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "Camera.h"
#include "Shader.h"
#include "Body.h"
#include "Mesh.h"
#include "TextureLoader.h"
#include "SolarSystem.h" 

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

CelestialBody* focusTarget = nullptr;
float followDistance = 200.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorLocked = true;
bool isPaused = false;
bool renderOrbits = true;

const float UNIFIED_RADIUS_SCALE = 0.00004f;

// --- OBS£UGA WEJŒCIA (Bez zmian) ---
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

    float currentSpeed = baseCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 5.0f;

    camera.MovementSpeed = currentSpeed;
    float dt = (float)deltaTimeDouble;

    if (focusTarget == nullptr) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP, dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, dt);
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) followDistance -= currentSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) followDistance += currentSpeed * dt;
        if (followDistance < 1.0f) followDistance = 1.0f;
    }

    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        cursorLocked = !cursorLocked;
        glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (cursorLocked) firstMouse = true;
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System - Final", NULL, NULL);
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

    // --- NOWOŒÆ: INICJALIZACJA UK£ADU S£ONECZNEGO Z KLASY ---
    SolarSystem solarSystem;
    solarSystem.initializeTextures();

    // £adowanie t³a (Skybox) zostawiamy w main, bo to element "Sceny", a nie fizyki planet
    unsigned int milkwayTex = loadTexture("milkway.jpg");

    // Konfiguracja shaderów
    planetShader.use();
    planetShader.setInt("diffuseTexture", 0);
    planetShader.setInt("nightTexture", 1);

    while (!glfwWindowShouldClose(window)) {
        double currentFrameDouble = glfwGetTime();
        deltaTimeDouble = currentFrameDouble - lastFrameDouble;
        lastFrameDouble = currentFrameDouble;
        simulationSpeed = timeMultiplier * (1.0 / 86400.0);

        processInput(window);

        if (!isPaused) {
            currentTimeDays += simulationSpeed * deltaTimeDouble;
            // Aktualizacja wszystkich planet jedn¹ komend¹
            solarSystem.update(currentTimeDays);
        }

        // Kamera w trybie Focus
        if (focusTarget != nullptr) {
            glm::vec3 targetPos = glm::vec3(focusTarget->worldPosition);
            camera.Position = targetPos - (camera.Front * followDistance);
        }

        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --- RYSOWANIE T£A ---
        planetShader.use();
        planetShader.setMat4("projection", projection);
        glm::mat4 viewSkybox = glm::mat4(glm::mat3(view));
        planetShader.setMat4("view", viewSkybox);
        planetShader.setBool("isSun", true);
        planetShader.setBool("hasTexture", true);
        planetShader.setBool("hasNightTexture", false);
        planetShader.setVec3("objectColor", glm::vec3(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, milkwayTex);
        glm::mat4 modelSky = glm::mat4(1.0f);
        modelSky = glm::scale(modelSky, glm::vec3(-5000.0f, -5000.0f, -5000.0f));
        planetShader.setMat4("model", modelSky);
        glDepthMask(GL_FALSE);
        glBindVertexArray(sphere.VAO);
        glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        // --- RYSOWANIE ORBIT ---
        if (renderOrbits) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);

            for (auto body : solarSystem.bodies) {
                if (body->parent == nullptr) continue; // S³oñce nie ma orbity

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

                // --- NOWOŒÆ: USTAWIENIE KOLORU ORBITY ---
                // U¿ywamy tego samego koloru co planeta, ale mo¿esz go np. przyciemniæ mno¿¹c razy 0.5f
                orbitShader.setVec3("orbitColor", body->color);

                glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)body->orbitPath.size());
            }
        }

        // --- RYSOWANIE PLANET ---
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", glm::vec3(0.0f)); // S³oñce jest w (0,0,0)
        planetShader.setVec3("viewPos", camera.Position);
        glBindVertexArray(sphere.VAO);

        for (auto body : solarSystem.bodies) {
            glm::mat4 model = glm::mat4(1.0f);
            glm::vec3 drawPos = glm::vec3(body->worldPosition);
            model = glm::translate(model, drawPos);
            float r_scale = (float)body->radius * UNIFIED_RADIUS_SCALE;

            planetShader.setVec3("objectColor", body->color);
            planetShader.setBool("isSun", (body->name == "Sun"));

            if (body->diffuseMap != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, body->diffuseMap);
                planetShader.setBool("hasTexture", true);
            }
            else {
                planetShader.setBool("hasTexture", false);
            }

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

        ImGui::Begin("Centrum Sterowania");
        ImGui::Text("Symulacja (Dzien: %.2f)", currentTimeDays);
        ImGui::Checkbox("Pauza", &isPaused);
        ImGui::Checkbox("Pokaz Orbity", &renderOrbits);
        ImGui::Separator();

        float timeMultFloat = (float)timeMultiplier;
        if (ImGui::SliderFloat("Predkosc Czasu", &timeMultFloat, 0.0f, 100000.0f, "%.0fx")) {
            timeMultiplier = (double)timeMultFloat;
        }

        ImGui::Separator();
        ImGui::Text("KAMERA");
        ImGui::SliderFloat("Predkosc Kamery", &baseCameraSpeed, 10.0f, 5000.0f);

        const char* currentFocusName = (focusTarget != nullptr) ? focusTarget->name.c_str() : "Wolna Kamera";
        if (ImGui::BeginCombo("Sledz Obiekt", currentFocusName)) {
            if (ImGui::Selectable("Wolna Kamera", focusTarget == nullptr)) {
                focusTarget = nullptr;
            }
            // Pobieramy listê planet z klasy SolarSystem
            for (auto body : solarSystem.bodies) {
                bool isSelected = (focusTarget == body);
                if (ImGui::Selectable(body->name.c_str(), isSelected)) {
                    focusTarget = body;
                    followDistance = (float)body->radius * UNIFIED_RADIUS_SCALE * 5.0f;
                    if (followDistance < 20.0f) followDistance = 20.0f;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (focusTarget != nullptr) {
            ImGui::Text("Sterowanie w trybie Focus:");
            ImGui::Text("- W / S: Przybliz / Oddal (Zoom)");
            ImGui::Text("- Myszka: Obrot wokol planety");
            ImGui::SliderFloat("Odleglosc (Zoom)", &followDistance, 5.0f, 1000.0f);
        }
        else {
            ImGui::Text("Sterowanie w trybie Wolnym:");
            ImGui::Text("- WASD: Latanie");
            ImGui::Text("- Shift: Przyspieszenie");
        }

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
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

// --- GLOBALS ---
double timeMultiplier = 1.0;
double simulationSpeed = 0.0;
double currentTimeDays = 0.0;
double deltaTimeDouble = 0.0;
double lastFrameDouble = 0.0;

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

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

// --- INPUT HANDLING ---
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
        // Focus Mode Logic
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
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabPressed = false;
    }
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

    // IMGUI Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- SHADER INIT ---
    Shader planetShader("planet.vert", "planet.frag");
    Shader orbitShader("orbit.vert", "orbit.frag");
    Shader ringShader("ring.vert", "ring.frag");

    // --- MESH INIT ---
    SphereMesh sphere = generateSphere(64, 64);
    // Rings for Saturn: Inner radius 1.2x, Outer 2.2x
    SphereMesh ringMesh = generateRing(1.2f, 2.2f, 128);

    // --- SOLAR SYSTEM INIT ---
    SolarSystem solarSystem;
    solarSystem.initializeTextures();

    // Skybox
    unsigned int milkwayTex = loadTexture("milkway.jpg");

    // Configure Shaders
    planetShader.use();
    planetShader.setInt("diffuseTexture", 0);
    planetShader.setInt("nightTexture", 1);

    ringShader.use();
    ringShader.setInt("ringTexture", 0);

    // --- MAIN LOOP ---
    while (!glfwWindowShouldClose(window)) {
        double currentFrameDouble = glfwGetTime();
        deltaTimeDouble = currentFrameDouble - lastFrameDouble;
        lastFrameDouble = currentFrameDouble;

        simulationSpeed = timeMultiplier * (1.0 / 86400.0);
        processInput(window);

        if (!isPaused) {
            currentTimeDays += simulationSpeed * deltaTimeDouble;
            solarSystem.update(currentTimeDays);
        }

        // Camera Update
        if (focusTarget != nullptr) {
            glm::vec3 targetPos = glm::vec3(focusTarget->worldPosition);
            camera.Position = targetPos - (camera.Front * followDistance);
        }

        // Render Setup
        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // 1. Draw Skybox
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

        // 2. Draw Orbits (FIXED LOOP)
        if (renderOrbits) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);

            for (auto body : solarSystem.bodies) {
                if (body->parent == nullptr) continue;

                // Initialize VAO only once
                if (body->VAO_Orbit == 0) {
                    glGenVertexArrays(1, &body->VAO_Orbit);
                    unsigned int VBO_Orbit;
                    glGenBuffers(1, &VBO_Orbit);

                    glBindVertexArray(body->VAO_Orbit);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO_Orbit);
                    glBufferData(GL_ARRAY_BUFFER, body->orbitPath.size() * sizeof(glm::vec3), &body->orbitPath[0], GL_STATIC_DRAW);

                    // ENABLE ATTRIBUTES HERE (Inside initialization)
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                }

                // For drawing, we just bind the VAO
                glBindVertexArray(body->VAO_Orbit);

                glm::mat4 model = glm::mat4(1.0f);
                if (body->parent) model = glm::translate(model, glm::vec3(body->parent->worldPosition));
                orbitShader.setMat4("model", model);
                orbitShader.setVec3("orbitColor", body->color * 0.5f);

                glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)body->orbitPath.size());
            }
        }

        // 3. Draw Planets
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", glm::vec3(0.0f));
        planetShader.setVec3("viewPos", camera.Position);

        glBindVertexArray(sphere.VAO);

        for (auto body : solarSystem.bodies) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(body->worldPosition));
            float r_scale = (float)body->radius * UNIFIED_RADIUS_SCALE;

            planetShader.setVec3("objectColor", body->color);
            planetShader.setBool("isSun", (body->name == "Sun"));

            // --- TEXTURE HANDLING (FIXED) ---
            // If texture ID is 0 (missing file), use False to render color instead of black
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

        // 4. Draw Rings
        ringShader.use();
        ringShader.setMat4("projection", projection);
        ringShader.setMat4("view", view);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glBindVertexArray(ringMesh.VAO);

        for (auto body : solarSystem.bodies) {
            if (body->ringMap == 0) continue;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(body->worldPosition));
            float r_scale = (float)body->radius * UNIFIED_RADIUS_SCALE;
            model = glm::scale(model, glm::vec3(r_scale));

            model = glm::rotate(model, glm::radians((float)body->axialTilt), glm::vec3(0, 0, 1));
            model = glm::rotate(model, glm::radians((float)body->currentRotationAngle), glm::vec3(0, 1, 0));

            ringShader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, body->ringMap);
            glDrawElements(GL_TRIANGLES, ringMesh.indexCount, GL_UNSIGNED_INT, 0);
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // 5. GUI
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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <string>
#include "Camera.h"
#include "Shader.h"
#include "Body.h"
#include "Mesh.h"
#include "TextureLoader.h"
#include "SolarSystem.h"
#include <algorithm> // Do sortowania
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
Camera camera(glm::vec3(0.0f, 300.0f, 700.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorLocked = false;
const float SYSTEM_BOUNDARY_RADIUS = 45000.0f;
const float WARNING_DISTANCE = 3000.0f;
bool gameStarted = false;
bool isPaused = false;
int orbitMode = 1;
CelestialBody* selectedBody = nullptr;
bool showInfoPanel = false;
int selectedMainAsteroidIdx = 0;
int selectedKuiperAsteroidIdx = 0;
struct BodyInfo {
    std::string type;
    std::string mass;
    std::string temp;
    std::string fact;
};
std::map<std::string, BodyInfo> knowledgeBase;
double timeMultiplier = 1.0;
double simulationSpeed = 0.0;
double currentTimeDays = 0.0;
double deltaTimeDouble = 0.0;
double lastFrameDouble = 0.0;
const float UNIFIED_RADIUS_SCALE = 0.00004f;
float baseCameraSpeed = 200.0f;
CelestialBody* focusTarget = nullptr;
float followDistance = 200.0f;
const char* tailVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in float aAlpha;\n"
"out float Alpha;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * vec4(aPos, 1.0);\n"
"   Alpha = aAlpha;\n"
"}\0";
const char* tailFragmentShaderSource = "#version 330 core\n"
"in float Alpha;\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(color, Alpha);\n"
"}\n\0";
const char* bloomVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"out vec3 FragPosLocal;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"void main()\n"
"{\n"
"    FragPosLocal = aPos;\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\0";
const char* bloomFragmentShaderSource = "#version 330 core\n"
"in vec3 FragPosLocal;\n"
"out vec4 FragColor;\n"
"uniform vec3 bloomColor;\n"
"uniform float intensity;\n"
"void main()\n"
"{\n"
"   float dist = length(FragPosLocal.xy);\n"
"   float alpha = 1.0 - smoothstep(0.0, 0.5, dist);\n"
"   alpha = pow(alpha, 2.5);\n"
"   FragColor = vec4(bloomColor, alpha * intensity);\n"
"}\n\0";
struct CometTrail {
    std::vector<glm::vec3> positions;
    unsigned int VAO = 0, VBO = 0;
};
std::map<std::string, CometTrail> cometTrails;
struct QuadMesh {
    unsigned int VAO, VBO;
};
QuadMesh generateQuad() {
    QuadMesh quad;
    float vertices[] = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
    };
    glGenVertexArrays(1, &quad.VAO);
    glGenBuffers(1, &quad.VBO);
    glBindVertexArray(quad.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    return quad;
}
void InitKnowledgeBase() {
    knowledgeBase["Sun"] = { "Gwiazda", "1.989 x 10^30 kg", "5500 C (Powierzchnia)", "Stanowi 99.86% masy calego Ukladu Slonecznego." };
    knowledgeBase["Mercury"] = { "Planeta", "3.285 x 10^23 kg", "-173 do 427 C", "Najmniejsza planeta, nie posiada atmosfery." };
    knowledgeBase["Venus"] = { "Planeta", "4.867 x 10^24 kg", "462 C", "Najgoretsza planeta przez efekt cieplarniany." };
    knowledgeBase["Earth"] = { "Planeta", "5.972 x 10^24 kg", "-88 do 58 C", "Jedyne znane miejsce we wszechswiecie z zyciem." };
    knowledgeBase["Mars"] = { "Planeta", "6.39 x 10^23 kg", "-87 do -5 C", "Czerwony kolor pochodzi od tlenku zelaza (rdzy)." };
    knowledgeBase["Jupiter"] = { "Gazowy Gigant", "1.898 x 10^27 kg", "-145 C", "Wielka Czerwona Plama to burza trwajaca od 300 lat." };
    knowledgeBase["Saturn"] = { "Gazowy Gigant", "5.683 x 10^26 kg", "-178 C", "Jego pierscienie skladaja sie glownie z lodu i skal." };
    knowledgeBase["Uranus"] = { "Lodowy Gigant", "8.681 x 10^25 kg", "-224 C", "Toczy sie po orbicie 'na boku' (os nachylona 98 stopni)." };
    knowledgeBase["Neptune"] = { "Lodowy Gigant", "1.024 x 10^26 kg", "-214 C", "Wieja tu najsilniejsze wiatry w ukladzie (2100 km/h)." };
    knowledgeBase["Pluto"] = { "Planeta Karlowata", "1.309 x 10^22 kg", "-229 C", "Zdegradowany do planety karlowatej w 2006 roku." };
    knowledgeBase["Moon"] = { "Ksiezyc (Ziemia)", "7.34 x 10^22 kg", "-173 do 127 C", "Powoduje plywy morskie na Ziemi." };
    knowledgeBase["Comet_Halley"] = { "Kometa", "~2.2 x 10^14 kg", "Zmienna", "Najslynniejsza kometa, widoczna z Ziemi co 75-76 lat." };
    knowledgeBase["Io"] = { "Ksiezyc (Jowisz)", "8.93 x 10^22 kg", "-143 C", "Najbardziej aktywny wulkanicznie obiekt w Ukladzie Slonecznym." };
    knowledgeBase["Europa"] = { "Ksiezyc (Jowisz)", "4.80 x 10^22 kg", "-160 C", "Posiada podlodowy ocean, potencjalne miejsce na zycie." };
    knowledgeBase["Ganymede"] = { "Ksiezyc (Jowisz)", "1.48 x 10^23 kg", "-163 C", "Najwiekszy ksiezyc w Ukladzie, wiekszy nawet od Merkurego." };
    knowledgeBase["Callisto"] = { "Ksiezyc (Jowisz)", "1.08 x 10^23 kg", "-139 C", "Najbardziej pokraterowany obiekt w Ukladzie Slonecznym." };
    knowledgeBase["Titan"] = { "Ksiezyc (Saturn)", "1.35 x 10^23 kg", "-179 C", "Jedyny ksiezyc z gesta atmosfera i jeziorami cieklego metanu." };
    knowledgeBase["Rhea"] = { "Ksiezyc (Saturn)", "2.30 x 10^21 kg", "-174 C", "Sklada sie w 75% z lodu wodnego, moze posiadac pierscienie." };
    knowledgeBase["Iapetus"] = { "Ksiezyc (Saturn)", "1.80 x 10^21 kg", "-143 C", "Ma dwie rozne polkule: jedna snieznobiala, druga czarna jak wegiel." };
    knowledgeBase["Dione"] = { "Ksiezyc (Saturn)", "1.10 x 10^21 kg", "-186 C", "Posiada charakterystyczne jasne smugi lodowe na powierzchni." };
    knowledgeBase["Tethys"] = { "Ksiezyc (Saturn)", "6.17 x 10^20 kg", "-187 C", "Posiada ogromny krater Odyseusz i wielki kanion Ithaca Chasma." };
    knowledgeBase["Titania"] = { "Ksiezyc (Uran)", "3.52 x 10^21 kg", "-213 C", "Najwiekszy ksiezyc Urana, pokryty brudnym lodem i kraterami." };
    knowledgeBase["Oberon"] = { "Ksiezyc (Uran)", "3.01 x 10^21 kg", "-213 C", "Najdalszy z duzych ksiezycow Urana, geologicznie stary." };
    knowledgeBase["Umbriel"] = { "Ksiezyc (Uran)", "1.20 x 10^21 kg", "-213 C", "Najciemniejszy z ksiezycow Urana, odbija bardzo malo swiatla." };
    knowledgeBase["Ariel"] = { "Ksiezyc (Uran)", "1.35 x 10^21 kg", "-213 C", "Najjasniejszy ksiezyc Urana, posiada najmlodsza powierzchnie." };
    knowledgeBase["Miranda"] = { "Ksiezyc (Uran)", "6.59 x 10^19 kg", "-213 C", "Wyglada jak 'posklejana'. Posiada najwyzszy klif w Ukladzie (20km)." };
    knowledgeBase["Triton"] = { "Ksiezyc (Neptun)", "2.14 x 10^22 kg", "-235 C", "Krazy w przeciwnym kierunku do obrotu planety (orbita wsteczna)." };
    knowledgeBase["Proteus"] = { "Ksiezyc (Neptun)", "4.4 x 10^19 kg", "-223 C", "Bardzo ciemny obiekt, jeden z najwiekszych niekulistych ksiezycow." };
    knowledgeBase["Nereid"] = { "Ksiezyc (Neptun)", "3.1 x 10^19 kg", "-223 C", "Posiada jedna z najbardziej wydluzonych orbit (ekscentryczna)." };
}
BodyInfo GetInfo(const std::string& name) {
    if (knowledgeBase.find(name) != knowledgeBase.end()) return knowledgeBase[name];
    if (name.find("Kuiper") != std::string::npos) return { "Obiekt Pasa Kuipera", "Zroznicowana", "~ -230 C", "Lodowy obiekt transneptunowy." };
    if (name.find("Asteroid") != std::string::npos) return { "Asteroida (Pas Glowny)", "~10^10 - 10^15 kg", "-73 C", "Skalisty obiekt miedzy Marsem a Jowiszem." };
    if (name.find("Comet") != std::string::npos) return { "Kometa", "Zmienna", "Zmienna", "Brudna sniezka z lodu i pylu." };
    return { "Cialo Niebieskie", "Nieznana", "Nieznana", "Brak szczegolowych danych w bazie." };
}
void CalculateRay(double mouseX, double mouseY, int screenW, int screenH, glm::mat4 view, glm::mat4 projection, glm::vec3& rayOrigin, glm::vec3& rayDirection) {
    float x = (2.0f * (float)mouseX) / (float)screenW - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / (float)screenH;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
    rayDirection = glm::normalize(ray_wor);
    rayOrigin = glm::vec3(glm::inverse(view)[3]);
}
bool CheckRaySphereIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float sphereRadius, float& intersectionDistance) {
    glm::vec3 oc = rayOrigin - sphereCenter;
    float b = glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float h = b * b - c;
    if (h < 0.0f) return false;
    float h_sqrt = sqrt(h);
    float t1 = -b - h_sqrt;
    float t2 = -b + h_sqrt;
    if (t1 > 0 && t1 < t2) { intersectionDistance = t1; return true; }
    if (t2 > 0) { intersectionDistance = t2; return true; }
    return false;
}
void ProcessSelection(SolarSystem& system, double mouseX, double mouseY, int width, int height, const glm::mat4& view, const glm::mat4& proj) {
    glm::vec3 rayOrigin, rayDir;
    CalculateRay(mouseX, mouseY, width, height, view, proj, rayOrigin, rayDir);
    float closestDist = 1000000.0f;
    CelestialBody* hitBody = nullptr;
    for (auto body : system.bodies) {
        float visualRadius = (float)body->radius * UNIFIED_RADIUS_SCALE;
        float hitBoxMult = 1.0f;
        if (body->name.find("Asteroid") != std::string::npos || body->name.find("Kuiper") != std::string::npos) hitBoxMult = 5.0f;
        else hitBoxMult = 1.1f;
        float dist;
        if (CheckRaySphereIntersection(rayOrigin, rayDir, body->worldPosition, visualRadius * hitBoxMult, dist)) {
            if (dist < closestDist) { closestDist = dist; hitBody = body; }
        }
    }
    if (hitBody != nullptr) {
        focusTarget = hitBody;
        selectedBody = hitBody;
        followDistance = (float)hitBody->radius * UNIFIED_RADIUS_SCALE * 4.0f;
        if (followDistance < 15.0f) followDistance = 15.0f;
        showInfoPanel = true;
    }
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!gameStarted || !cursorLocked) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
SolarSystem* globalSolarSystemPtr = nullptr;
glm::mat4 globalProjection;
glm::mat4 globalView;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && gameStarted) {
        if (ImGui::GetIO().WantCaptureMouse) return;
        double xpos, ypos;
        if (cursorLocked) { xpos = SCR_WIDTH / 2.0; ypos = SCR_HEIGHT / 2.0; }
        else { glfwGetCursorPos(window, &xpos, &ypos); }
        if (globalSolarSystemPtr) {
            ProcessSelection(*globalSolarSystemPtr, xpos, ypos, SCR_WIDTH, SCR_HEIGHT, globalView, globalProjection);
        }
    }
}
void processInput(GLFWwindow* window) {
    static bool escPressedLastFrame = false;
    bool escPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escPressed && !escPressedLastFrame) {
        if (gameStarted) {
            gameStarted = false;
            cursorLocked = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            focusTarget = nullptr;
            selectedBody = nullptr;
            showInfoPanel = false;
        }
        else {
            glfwSetWindowShouldClose(window, true);
        }
    }
    escPressedLastFrame = escPressed;
    if (!gameStarted) return;
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
        if (focusTarget != nullptr) {
            float visualRadius = (float)focusTarget->radius * UNIFIED_RADIUS_SCALE;
            float minZoom = visualRadius * 1.1f + 2.0f;
            if (followDistance < minZoom) { followDistance = minZoom; }
        }
    }
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        cursorLocked = !cursorLocked;
        glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (cursorLocked) firstMouse = true;
        tabPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) { tabPressed = false; }
}
void TextCentered(const char* text) {
    float win_width = ImGui::GetWindowSize().x;
    float text_width = ImGui::CalcTextSize(text).x;
    float text_indent = (win_width - text_width) * 0.5f;
    if (text_indent > 0.0f) ImGui::SetCursorPosX(text_indent);
    ImGui::Text("%s", text);
}
void RenderStartScreen(int width, int height) {
    float time = (float)glfwGetTime();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("StartScreen", nullptr, window_flags);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ImVec2(0, 0), ImVec2((float)width, (float)height), IM_COL32(0, 0, 0, 150));
    if (time > 2.0f) {
        float animTime = time - 2.0f;
        float speed = 900.0f;
        float startX = (float)width + 100.0f;
        float currentX = startX - (animTime * speed);
        float currentY = height * 0.2f + sin(animTime * 2.0f) * 50.0f;
        if (currentX > -200.0f) {
            ImVec2 center(currentX, currentY);
            float radius = 40.0f;
            draw_list->AddCircleFilled(center, radius, IM_COL32(100, 100, 100, 255));
            draw_list->AddCircleFilled(ImVec2(currentX - 10, currentY - 10), 12.0f, IM_COL32(60, 60, 60, 200));
            draw_list->AddCircleFilled(ImVec2(currentX + 15, currentY + 5), 8.0f, IM_COL32(60, 60, 60, 200));
            draw_list->AddCircleFilled(ImVec2(currentX - 5, currentY + 20), 5.0f, IM_COL32(60, 60, 60, 200));
            draw_list->AddTriangleFilled(
                ImVec2(currentX + radius, currentY - 10),
                ImVec2(currentX + radius, currentY + 10),
                ImVec2(currentX + radius + 100 + (sin(time * 20.0f) * 20.0f), currentY),
                IM_COL32(255, 100, 0, 150)
            );
        }
    }
    ImGui::SetCursorPosY(height * 0.3f);
    const char* titleText = "SOLAR SYSTEM SIMULATOR";
    float baseScale = 3.0f;
    float pulse = (sin(time * 3.0f) * 0.1f);
    float currentScale = baseScale + pulse;
    float colorLerp = (sin(time * 1.5f) + 1.0f) * 0.5f;
    ImVec4 titleColor = ImVec4(1.0f, 0.8f + (0.2f * colorLerp), 0.2f + (0.8f * colorLerp), 1.0f);
    ImGui::SetWindowFontScale(currentScale);
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextColored(titleColor, "%s", titleText);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SetCursorPosY(height * 0.48f);
    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.7f, 1.0f));
    ImGui::SetWindowFontScale(1.5f);
    if (ImGui::Button("START SYMULACJI", ImVec2(buttonWidth, buttonHeight))) {
        gameStarted = true;
        cursorLocked = true;
        glm::vec3 direction = camera.Front;
        camera.Yaw = glm::degrees(atan2(direction.z, direction.x));
        camera.Pitch = glm::degrees(asin(direction.y));
        GLFWwindow* currWindow = glfwGetCurrentContext();
        glfwSetInputMode(currWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(3);
    float footerHeight = 150.0f;
    ImGui::SetCursorPosY(height - footerHeight);
    draw_list->AddRectFilled(ImVec2(0, (float)height - footerHeight), ImVec2((float)width, (float)height), IM_COL32(20, 20, 30, 200));
    ImGui::Separator();
    TextCentered("STEROWANIE");
    ImGui::Separator();
    ImGui::Columns(3, "controls", false);
    ImGui::SetColumnWidth(0, width * 0.43f);
    ImGui::SetColumnWidth(1, width * 0.43f);
    ImGui::SetColumnWidth(2, width * 0.43f);
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "RUCH:");
    ImGui::BulletText("W, A, S, D - Latanie");
    ImGui::BulletText("Mysz - Rozgladanie");
    ImGui::BulletText("Spacja / CTRL - Gora / Dol");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "FUNKCJE:");
    ImGui::BulletText("LPM - Wybierz planete");
    ImGui::BulletText("C - Przyblizenie (Teleskop)");
    ImGui::BulletText("SHIFT - Przyspieszenie");
    ImGui::BulletText("TAB - Odblokuj kursor");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "SYSTEM:");
    ImGui::BulletText("Pauza - Wlacz / Wylacz");
    ImGui::BulletText("ESC - Wyjscie");
    ImGui::Columns(1);
    ImGui::End();
}
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System - Ultimate", primaryMonitor, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    unsigned int bloomVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(bloomVS, 1, &bloomVertexShaderSource, NULL);
    glCompileShader(bloomVS);
    unsigned int bloomFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(bloomFS, 1, &bloomFragmentShaderSource, NULL);
    glCompileShader(bloomFS);
    unsigned int bloomShaderProgram = glCreateProgram();
    glAttachShader(bloomShaderProgram, bloomVS);
    glAttachShader(bloomShaderProgram, bloomFS);
    glLinkProgram(bloomShaderProgram);
    glDeleteShader(bloomVS);
    glDeleteShader(bloomFS);
    QuadMesh bloomQuad = generateQuad();
    float bloomSize = 50000.0f;
    glm::vec3 bloomColor = glm::vec3(1.0f, 0.8f, 0.4f);
    float bloomIntensity = 0.05f;
    InitKnowledgeBase();
    Shader planetShader("planet.vert", "planet.frag");
    Shader orbitShader("orbit.vert", "orbit.frag");
    Shader ringShader("ring.vert", "ring.frag");
    unsigned int tailVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(tailVertexShader, 1, &tailVertexShaderSource, NULL);
    glCompileShader(tailVertexShader);
    unsigned int tailFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(tailFragmentShader, 1, &tailFragmentShaderSource, NULL);
    glCompileShader(tailFragmentShader);
    unsigned int tailShaderProgram = glCreateProgram();
    glAttachShader(tailShaderProgram, tailVertexShader);
    glAttachShader(tailShaderProgram, tailFragmentShader);
    glLinkProgram(tailShaderProgram);
    glDeleteShader(tailVertexShader);
    glDeleteShader(tailFragmentShader);
    SphereMesh sphere = generateSphere(64, 64);
    SphereMesh ringMesh = generateRing(1.2f, 2.2f, 128);
    std::vector<SphereMesh> asteroidMeshes;
    int minPoly = 15;
    int maxPoly = 25;
    for (int i = 0; i < 20; i++) {
        int sectors = minPoly + (rand() % (maxPoly - minPoly + 1));
        int stacks = minPoly + (rand() % (maxPoly - minPoly + 1));
        asteroidMeshes.push_back(generateSphere(sectors, stacks));
    }
    SolarSystem solarSystem;
    solarSystem.initializeTextures();
    globalSolarSystemPtr = &solarSystem;
    unsigned int milkwayTex = loadTexture("milkway.jpg");
    unsigned int detailTex = loadTexture("stars_detail.jpg");
    unsigned int kuiperTex = loadTexture("kuiper.jpg");
    CelestialBody* sunPtr = nullptr;
    for (auto b : solarSystem.bodies) { if (b->name == "Sun") { sunPtr = b; break; } }
    srand(1234);
    for (int i = 0; i < 100; i++) {
        CelestialBody* ast = new CelestialBody();
        ast->name = "Obiekt Pasa Planetoid " + std::to_string(i);
        float scaleVar = ((rand() % 100) / 100.0f) * 0.3f + 0.1f;
        ast->radius = 500.0 * scaleVar;
        ast->color = glm::vec3(0.6f, 0.55f, 0.5f);
        double distAU = 2.2 + ((rand() % 100) / 100.0) * 1.0;
        ast->orbit.semiMajorAxis = distAU;
        ast->orbit.eccentricity = (rand() % 20) / 100.0;
        ast->orbit.inclination = (rand() % 10) - 5.0;
        ast->orbit.meanAnomalyEpoch = rand() % 360;
        ast->orbit.orbitalPeriod = sqrt(pow(distAU, 3.0));
        ast->parent = sunPtr;
        if (sunPtr) sunPtr->children.push_back(ast);
        ast->axialTilt = (rand() % 360);
        ast->generateFullOrbit(64);
        solarSystem.bodies.push_back(ast);
    }
    for (int i = 0; i < 500; i++) {
        CelestialBody* kObj = new CelestialBody();
        kObj->name = "Obiekt Pasa Kuipera " + std::to_string(i);
        float scaleVar = ((rand() % 100) / 100.0f) * 0.7f + 0.5f;
        kObj->radius = 3000.0 * scaleVar;
        float blueish = ((rand() % 50) / 100.0f);
        kObj->color = glm::vec3(0.5f, 0.5f + blueish, 0.6f + blueish);
        kObj->diffuseMap = kuiperTex;
        double minKuiperAU = 35.0;
        double maxKuiperAU = 55.0;
        double distAU = minKuiperAU + (rand() % (int)((maxKuiperAU - minKuiperAU) * 100)) / 100.0;
        kObj->orbit.semiMajorAxis = distAU;
        kObj->orbit.eccentricity = (rand() % 200) / 1000.0;
        float baseInc = (rand() % 100) / 10.0f;
        if (rand() % 5 == 0) baseInc += 15.0f;
        kObj->orbit.inclination = (rand() % 2 == 0 ? 1 : -1) * baseInc;
        kObj->orbit.meanAnomalyEpoch = rand() % 360;
        kObj->orbit.orbitalPeriod = sqrt(pow(distAU, 3.0));
        kObj->parent = sunPtr;
        if (sunPtr) sunPtr->children.push_back(kObj);
        kObj->axialTilt = (rand() % 360);
        kObj->generateFullOrbit(64);
        solarSystem.bodies.push_back(kObj);
    }
    planetShader.use();
    planetShader.setInt("diffuseTexture", 0);
    planetShader.setInt("detailTexture", 1);
    planetShader.setInt("nightTexture", 2);
    ringShader.use();
    ringShader.setInt("ringTexture", 0);
    float boundaryWarningAlpha = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        double currentFrameDouble = glfwGetTime();
        deltaTimeDouble = currentFrameDouble - lastFrameDouble;
        lastFrameDouble = currentFrameDouble;
        float dt = (float)deltaTimeDouble;
        if (!gameStarted) {
            float camX = sin((float)glfwGetTime() * 0.1f) * 800.0f;
            float camZ = cos((float)glfwGetTime() * 0.1f) * 800.0f;
            camera.Position = glm::vec3(camX, 200.0f, camZ);
            camera.Front = glm::normalize(glm::vec3(0.0f) - camera.Position);
        }
        simulationSpeed = timeMultiplier * (1.0 / 86400.0);
        processInput(window);
        if (!isPaused) {
            currentTimeDays += simulationSpeed * deltaTimeDouble;
            solarSystem.update(currentTimeDays);
        }
        if (focusTarget != nullptr && gameStarted) {
            glm::vec3 targetPos = glm::vec3(focusTarget->worldPosition);
            camera.Position = targetPos - (camera.Front * followDistance);
        }
        for (auto body : solarSystem.bodies) {
            float visualRadius = (float)body->radius * UNIFIED_RADIUS_SCALE;
            float collisionDist = visualRadius * 1.05f + 1.0f;
            float distToBody = glm::length(camera.Position - glm::vec3(body->worldPosition));
            if (distToBody < collisionDist) {
                glm::vec3 dirFromCenter = glm::normalize(camera.Position - glm::vec3(body->worldPosition));
                camera.Position = glm::vec3(body->worldPosition) + (dirFromCenter * collisionDist);
                if (focusTarget != nullptr) {
                    glm::vec3 targetPos = glm::vec3(focusTarget->worldPosition);
                    followDistance = glm::distance(camera.Position, targetPos);
                }
            }
        }
        if (focusTarget == nullptr) {
            for (auto body : solarSystem.bodies) {
                float visualRadius = (float)body->radius * UNIFIED_RADIUS_SCALE;
                float minDistance = visualRadius * 1.05f + 1.0f;
                float dist = glm::length(camera.Position - glm::vec3(body->worldPosition));
                if (dist < minDistance) {
                    glm::vec3 directionFromCenter = glm::normalize(camera.Position - glm::vec3(body->worldPosition));
                    camera.Position = glm::vec3(body->worldPosition) + (directionFromCenter * minDistance);
                }
            }
        }
        float distFromSun = glm::length(camera.Position);
        if (distFromSun > SYSTEM_BOUNDARY_RADIUS) {
            camera.Position = glm::normalize(camera.Position) * SYSTEM_BOUNDARY_RADIUS;
            if (focusTarget != nullptr) {
                glm::vec3 targetPos = glm::vec3(focusTarget->worldPosition);
                followDistance = glm::distance(camera.Position, targetPos);
            }
        }
        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        float currentSpeed = camera.MovementSpeed;
        float speedRatio = 0.0f;
        if (currentSpeed > 2000.0f) {
            speedRatio = (currentSpeed - 2000.0f) / 8000.0f;
            speedRatio = 0.5f + (speedRatio * 0.5f);
        }
        else if (currentSpeed > 500.0f) {
            speedRatio = (currentSpeed - 500.0f) / 1500.0f;
            speedRatio *= 0.5f;
        }
        speedRatio = glm::clamp(speedRatio, 0.0f, 1.0f);
        float targetFOV;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) targetFOV = 10.0f;
        else targetFOV = 45.0f + (55.0f * speedRatio);
        static float currentFOV = 45.0f;
        currentFOV += (targetFOV - currentFOV) * 10.0f * (float)deltaTimeDouble;
        glm::mat4 projection = glm::perspective(glm::radians(currentFOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        globalProjection = projection;
        globalView = view;
        float eclipseFactor = 1.0f;
        const float SUN_ANGULAR_RADIUS_DEG = 2.5f;
        float sunAngularRadius = glm::radians(SUN_ANGULAR_RADIUS_DEG);
        glm::vec3 camPos = camera.Position;
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f) - camPos);
        for (auto body : solarSystem.bodies) {
            if (body->name == "Sun" ||
                body->name.find("Asteroid") != std::string::npos ||
                body->name.find("Kuiper") != std::string::npos ||
                body->name.find("Comet") != std::string::npos) continue;
            glm::vec3 bodyPos = glm::vec3(body->worldPosition);
            glm::vec3 camToBody = bodyPos - camPos;
            float distToBody = glm::length(camToBody);
            float distToSun = glm::length(glm::vec3(0.0f) - camPos);
            if (distToBody > distToSun) continue;
            float bodyVisualRadius = (float)body->radius * UNIFIED_RADIUS_SCALE;
            float bodyAngularRadius = atan(bodyVisualRadius / distToBody);
            glm::vec3 bodyDir = glm::normalize(camToBody);
            float dotProd = glm::dot(sunDir, bodyDir);
            if (dotProd > 1.0f) dotProd = 1.0f;
            float separationAngle = acos(dotProd);
            if (separationAngle < (sunAngularRadius + bodyAngularRadius)) {
                if (bodyAngularRadius >= sunAngularRadius &&
                    separationAngle <= (bodyAngularRadius - sunAngularRadius)) {
                    eclipseFactor = 0.0f;
                }
                else {
                    float startOverlap = sunAngularRadius + bodyAngularRadius;
                    float fullOverlap = fabs(bodyAngularRadius - sunAngularRadius);
                    float coverage = (startOverlap - separationAngle) / (startOverlap - fullOverlap);
                    float maxDarkness = 0.0f;
                    if (bodyAngularRadius < sunAngularRadius) {
                        float areaRatio = (bodyAngularRadius * bodyAngularRadius) / (sunAngularRadius * sunAngularRadius);
                        maxDarkness = 1.0f - areaRatio;
                    }
                    float currentIntensity = 1.0f - coverage;
                    if (currentIntensity < maxDarkness) currentIntensity = maxDarkness;
                    eclipseFactor = std::min(eclipseFactor, currentIntensity);
                }
            }
        }
        if (eclipseFactor < 0.01f) eclipseFactor = 0.01f;
        planetShader.use();
        planetShader.setFloat("eclipseFactor", eclipseFactor);
        planetShader.setFloat("time", (float)glfwGetTime());
        planetShader.setFloat("fov", currentFOV);
        planetShader.setMat4("projection", projection);
        glm::mat4 viewSkybox = glm::mat4(glm::mat3(view));
        planetShader.setMat4("view", viewSkybox);
        planetShader.setBool("isSun", true);
        planetShader.setBool("hasTexture", true);
        planetShader.setBool("hasNightTexture", false);
        planetShader.setVec3("objectColor", glm::vec3(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, milkwayTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, detailTex);
        glm::mat4 modelSky = glm::mat4(1.0f);
        modelSky = glm::scale(modelSky, glm::vec3(-5000.0f, -5000.0f, -5000.0f));
        planetShader.setMat4("model", modelSky);
        glDepthMask(GL_FALSE);
        glBindVertexArray(sphere.VAO);
        glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        if (orbitMode != 0) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            for (auto body : solarSystem.bodies) {
                if (body->parent == nullptr) continue;
                bool isKuiper = (body->name.find("Kuiper") != std::string::npos);
                bool isAsteroid = (body->name.find("Asteroid") != std::string::npos);
                bool isComet = (body->name.find("Comet") != std::string::npos);
                bool isSmallBody = isKuiper || isAsteroid || isComet;
                if (orbitMode == 1 && isSmallBody) continue;
                if (orbitMode == 2 && !isSmallBody) continue;
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
                if (isKuiper) orbitShader.setVec4("orbitColor", glm::vec4(0.1f, 0.3f, 0.5f, 0.05f));
                else if (isAsteroid) orbitShader.setVec4("orbitColor", glm::vec4(body->color, 0.15f));
                else if (isSmallBody) orbitShader.setVec4("orbitColor", glm::vec4(body->color, 0.3f));
                else orbitShader.setVec4("orbitColor", glm::vec4(body->color, 0.5f));
                glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)body->orbitPath.size());
            }
            glDisable(GL_BLEND);
        }
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", glm::vec3(0.0f));
        planetShader.setVec3("viewPos", camera.Position);
        int bodyIndex = 0;
        for (auto body : solarSystem.bodies) {
            int currentIndices = 0;
            if (body->name.find("Asteroid") != std::string::npos || body->name.find("Kuiper") != std::string::npos) {
                int shapeVariant = bodyIndex % asteroidMeshes.size();
                glBindVertexArray(asteroidMeshes[shapeVariant].VAO);
                currentIndices = asteroidMeshes[shapeVariant].indexCount;
                planetShader.setBool("hasNightTexture", false);
            }
            else {
                glBindVertexArray(sphere.VAO);
                currentIndices = sphere.indexCount;
            }
            bodyIndex++;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(body->worldPosition));
            model = glm::rotate(model, glm::radians((float)body->axialTilt), glm::vec3(0, 0, 1));
            model = glm::rotate(model, glm::radians((float)body->currentRotationAngle), glm::vec3(0, 1, 0));
            float r_scale = (float)body->radius * UNIFIED_RADIUS_SCALE;
            float flattening = 1.0f;
            bool needsTextureCorrection = (body->name == "Jupiter" || body->name == "Saturn");
            if (body->name == "Earth") flattening = 0.99664f;
            else if (body->name == "Mars") flattening = 0.99412f;
            else if (body->name == "Jupiter") flattening = 0.93513f;
            else if (body->name == "Saturn") flattening = 0.90206f;
            else if (body->name == "Uranus") flattening = 0.97711f;
            else if (body->name == "Neptune") flattening = 0.98283f;
            if (needsTextureCorrection) {
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, glm::vec3(r_scale, r_scale, r_scale * flattening));
            }
            else {
                model = glm::scale(model, glm::vec3(r_scale, r_scale * flattening, r_scale));
            }
            bool hasAtmo = false;
            glm::vec3 atmoColor = glm::vec3(0.0f);
            if (body->name == "Earth") { hasAtmo = true; atmoColor = glm::vec3(0.3f, 0.6f, 1.0f); }
            else if (body->name == "Venus") { hasAtmo = true; atmoColor = glm::vec3(0.9f, 0.7f, 0.5f); }
            else if (body->name == "Mars") { hasAtmo = true; atmoColor = glm::vec3(0.8f, 0.3f, 0.1f); }
            planetShader.setBool("hasAtmosphere", hasAtmo);
            planetShader.setVec3("atmosphereColorRGB", atmoColor);
            planetShader.setVec3("objectColor", body->color);
            bool isSunObj = (body->name == "Sun");
            planetShader.setBool("isSun", isSunObj);
            if (isSunObj) planetShader.setVec3("objectColor", glm::vec3(1.0f));
            if (body->diffuseMap != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, body->diffuseMap);
                planetShader.setBool("hasTexture", true);
            }
            else { planetShader.setBool("hasTexture", false); }
            if (body->nightMap != 0) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, body->nightMap);
                planetShader.setBool("hasNightTexture", true);
            }
            else if (!isSunObj) { planetShader.setBool("hasNightTexture", false); }
            planetShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, currentIndices, GL_UNSIGNED_INT, 0);
        }
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
        glUseProgram(tailShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(tailShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(tailShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniform3f(glGetUniformLocation(tailShaderProgram, "color"), 0.6f, 0.8f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (auto body : solarSystem.bodies) {
            if (body->name.find("Comet") == std::string::npos) continue;
            CometTrail& trail = cometTrails[body->name];
            if (timeMultiplier > 100000.0) { trail.positions.clear(); continue; }
            glm::vec3 currentPos = body->worldPosition;
            if (trail.positions.empty()) {
                trail.positions.push_back(currentPos);
            }
            else {
                glm::vec3 lastPos = trail.positions.back();
                float dist = glm::distance(lastPos, currentPos);
                if (dist > 0.5f && dist < 1000.0f) {
                    int steps = (int)(dist * 2.0f);
                    if (steps > 100) steps = 100;
                    for (int i = 1; i <= steps; ++i) {
                        float t = (float)i / (float)steps;
                        trail.positions.push_back(glm::mix(lastPos, currentPos, t));
                    }
                }
                else if (dist >= 1000.0f) {
                    trail.positions.clear(); trail.positions.push_back(currentPos);
                }
                else { trail.positions.push_back(currentPos); }
            }
            if (trail.positions.size() > 400) {
                size_t removeCount = trail.positions.size() - 400;
                trail.positions.erase(trail.positions.begin(), trail.positions.begin() + removeCount);
            }
            if (trail.positions.size() < 2) continue;
            std::vector<float> trailData;
            for (size_t i = 0; i < trail.positions.size(); ++i) {
                trailData.push_back(trail.positions[i].x);
                trailData.push_back(trail.positions[i].y);
                trailData.push_back(trail.positions[i].z);
                float ratio = (float)i / (float)trail.positions.size();
                trailData.push_back(ratio * ratio);
            }
            if (trail.VAO == 0) { glGenVertexArrays(1, &trail.VAO); glGenBuffers(1, &trail.VBO); }
            glBindVertexArray(trail.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, trail.VBO);
            glBufferData(GL_ARRAY_BUFFER, trailData.size() * sizeof(float), trailData.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glLineWidth(3.0f);
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)trail.positions.size());
        }
        glDisable(GL_BLEND);
        glUseProgram(bloomShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(bloomShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(bloomShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniform3f(glGetUniformLocation(bloomShaderProgram, "bloomColor"), bloomColor.x, bloomColor.y, bloomColor.z);
        glUniform1f(glGetUniformLocation(bloomShaderProgram, "intensity"), bloomIntensity * eclipseFactor);
        glm::mat4 modelBloom = glm::mat4(1.0f);
        modelBloom = glm::translate(modelBloom, glm::vec3(0.0f, 0.0f, 0.0f));
        modelBloom[0][0] = view[0][0];
        modelBloom[0][1] = view[1][0];
        modelBloom[0][2] = view[2][0];
        modelBloom[1][0] = view[0][1];
        modelBloom[1][1] = view[1][1];
        modelBloom[1][2] = view[2][1];
        modelBloom[2][0] = view[0][2];
        modelBloom[2][1] = view[1][2];
        modelBloom[2][2] = view[2][2];
        modelBloom = glm::scale(modelBloom, glm::vec3(bloomSize));
        glUniformMatrix4fv(glGetUniformLocation(bloomShaderProgram, "model"), 1, GL_FALSE, &modelBloom[0][0]);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glBindVertexArray(bloomQuad.VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (!gameStarted) {
            RenderStartScreen(SCR_WIDTH, SCR_HEIGHT);
        }
        else {
            ImGui::Begin("Centrum Sterowania");
            int years = (int)(currentTimeDays / 365.0);
            float days = (float)fmod(currentTimeDays, 365.0);
            ImGui::Text("Rok: %d   Dzien: %.1f", years, days);
            ImGui::Checkbox("Pauza", &isPaused);
            const char* orbitOptions[] = { "Ukryte", "Planety + karlowate", "Asteroidy+", "Wszystko" };
            ImGui::Combo("Widok Orbit", &orbitMode, orbitOptions, IM_ARRAYSIZE(orbitOptions));
            ImGui::Separator();
            float timeMultFloat = (float)timeMultiplier;
            if (ImGui::SliderFloat("Predkosc Czasu", &timeMultFloat, 0.0f, 10000000.0f, "%.0fx")) timeMultiplier = (double)timeMultFloat;
            if (ImGui::Button("Resetuj Date")) {
                currentTimeDays = 0.0;
                isPaused = true;
                solarSystem.update(currentTimeDays);
            }
            ImGui::Separator();
            ImGui::Text("KAMERA");
            ImGui::SliderFloat("Predkosc Kamery", &baseCameraSpeed, 10.0f, 5000.0f);
            const char* currentFocusName = (focusTarget != nullptr) ? focusTarget->name.c_str() : "Wolna Kamera";
            if (ImGui::BeginCombo("Sledz Obiekt", currentFocusName)) {
                if (ImGui::Selectable("Wolna Kamera", focusTarget == nullptr)) {
                    focusTarget = nullptr; selectedBody = nullptr; showInfoPanel = false;
                }
                for (auto body : solarSystem.bodies) {
                    if (body->name.find("Obiekt Pasa") != std::string::npos) continue;
                    if (body->name.find("Obiekt Pasu") != std::string::npos) continue;
                    if (body->name.find("Asteroid") != std::string::npos) continue;
                    bool isSelected = (focusTarget == body);
                    if (ImGui::Selectable(body->name.c_str(), isSelected)) {
                        focusTarget = body; selectedBody = body; showInfoPanel = true;
                        followDistance = (float)body->radius * UNIFIED_RADIUS_SCALE * 5.0f;
                        if (followDistance < 20.0f) followDistance = 20.0f;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "NAWIGACJA ASTEROID");
            if (ImGui::InputInt("Nr (0-99)##Main", &selectedMainAsteroidIdx, 1, 10)) {
                if (selectedMainAsteroidIdx < 0) selectedMainAsteroidIdx = 0;
                if (selectedMainAsteroidIdx > 99) selectedMainAsteroidIdx = 99;
            }
            ImGui::SameLine();
            if (ImGui::Button("Namierz##Main")) {
                std::string targetName = "Obiekt Pasa Planetoid " + std::to_string(selectedMainAsteroidIdx);
                bool found = false;
                for (auto body : solarSystem.bodies) {
                    if (body->name == targetName) {
                        focusTarget = body;
                        selectedBody = body;
                        showInfoPanel = true;
                        followDistance = 30.0f;
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << "BLAD: Nie znaleziono: " << targetName << std::endl;
            }
            if (ImGui::InputInt("Nr (0-499)##Kuiper", &selectedKuiperAsteroidIdx, 1, 10)) {
                if (selectedKuiperAsteroidIdx < 0) selectedKuiperAsteroidIdx = 0;
                if (selectedKuiperAsteroidIdx > 499) selectedKuiperAsteroidIdx = 499;
            }
            ImGui::SameLine();
            if (ImGui::Button("Namierz##Kuiper")) {
                std::string targetName = "Obiekt Pasa Kuipera " + std::to_string(selectedKuiperAsteroidIdx);
                bool found = false;
                for (auto body : solarSystem.bodies) {
                    if (body->name == targetName) {
                        focusTarget = body;
                        selectedBody = body;
                        showInfoPanel = true;
                        followDistance = 50.0f;
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << "BLAD: Nie znaleziono: " << targetName << std::endl;
            }
            ImGui::End();
            if (showInfoPanel && selectedBody != nullptr) {
                ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 320.0f, 20.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(300.0f, 250.0f));
                std::string title = "Dane: " + selectedBody->name;
                if (ImGui::Begin(title.c_str(), &showInfoPanel)) {
                    BodyInfo info = GetInfo(selectedBody->name);
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Typ:"); ImGui::SameLine(); ImGui::Text("%s", info.type.c_str());
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Masa:"); ImGui::Text("%s", info.mass.c_str());
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Temperatura:"); ImGui::Text("%s", info.temp.c_str());
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Ciekawostka:"); ImGui::TextWrapped("%s", info.fact.c_str());
                    ImGui::Separator();
                    if (ImGui::Button("Zamknij")) showInfoPanel = false;
                }
                ImGui::End();
            }
            if (cursorLocked) {
                ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
                draw_list->AddCircle(ImVec2(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f), 3.0f, IM_COL32(255, 255, 255, 100), 12, 2.0f);
            }
            float camDist = glm::length(camera.Position);
            float targetAlpha = 0.0f;
            if (camDist > (SYSTEM_BOUNDARY_RADIUS - WARNING_DISTANCE)) {
                targetAlpha = 1.0f;
            }
            float fadeSpeed = 2.0f;
            if (boundaryWarningAlpha < targetAlpha) {
                boundaryWarningAlpha += fadeSpeed * dt;
                if (boundaryWarningAlpha > 1.0f) boundaryWarningAlpha = 1.0f;
            }
            else if (boundaryWarningAlpha > targetAlpha) {
                boundaryWarningAlpha -= fadeSpeed * dt;
                if (boundaryWarningAlpha < 0.0f) boundaryWarningAlpha = 0.0f;
            }
            if (boundaryWarningAlpha > 0.01f) {
                ImGui::SetNextWindowPos(ImVec2(0, SCR_HEIGHT * 0.4f));
                ImGui::SetNextWindowSize(ImVec2((float)SCR_WIDTH, 200.0f));
                ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
                ImGui::Begin("BoundaryWarning", nullptr, flags);
                ImGui::SetWindowFontScale(3.0f);
                const char* warnText = "GRANICA UKLADU SLONECZNEGO";
                float txtW = ImGui::CalcTextSize(warnText).x;
                ImGui::SetCursorPosX((SCR_WIDTH - txtW) * 0.5f);
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, boundaryWarningAlpha), "%s", warnText);
                ImGui::SetWindowFontScale(1.0f);
                ImGui::End();
            }
        }
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
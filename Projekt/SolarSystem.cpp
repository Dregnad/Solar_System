#include "SolarSystem.h"
#include "TextureLoader.h" // U¿ywamy naszego loadera
#include <glm/glm.hpp>

SolarSystem::SolarSystem() {
    // Tworzenie obiektów na stercie (u¿ywaj¹c 'new'), aby ¿y³y wewn¹trz klasy
    // UWAGA: Kolejnoœæ dodawania do wektora jest wa¿na dla pêtli w main

    CelestialBody* sun = new CelestialBody("Sun", 696340, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, 25.0, 7.25, nullptr);
    sun->color = glm::vec3(1.0f, 0.9f, 0.0f);
    bodies.push_back(sun);

    CelestialBody* mercury = new CelestialBody("Mercury", 2440, { 0.387, 0.2056, 7.00, 48.33, 29.12, 174.79, 0.24 }, 58.6, 0.03, sun);
    mercury->color = glm::vec3(0.6f, 0.6f, 0.6f);
    bodies.push_back(mercury);

    CelestialBody* venus = new CelestialBody("Venus", 6052, { 0.723, 0.0067, 3.39, 76.68, 54.88, 50.11, 0.615 }, 243.0, 177.3, sun);
    venus->color = glm::vec3(0.9f, 0.8f, 0.6f);
    bodies.push_back(venus);

    CelestialBody* earth = new CelestialBody("Earth", 6371, { 1.000, 0.0167, 0.000, -11.26, 102.94, 100.46, 1.0 }, 1.0, 23.5, sun);
    earth->color = glm::vec3(0.0f, 0.4f, 0.8f);
    bodies.push_back(earth);

    CelestialBody* moon = new CelestialBody("Moon", 1737, { 0.00257, 0.0549, 5.145, 125.08, 318.15, 135.27, 0.0748 }, 27.3, 6.7, earth);
    moon->color = glm::vec3(0.7f, 0.7f, 0.7f);
    bodies.push_back(moon);

    CelestialBody* mars = new CelestialBody("Mars", 3390, { 1.524, 0.0934, 1.85, 49.58, 286.50, 19.37, 1.88 }, 1.03, 25.2, sun);
    mars->color = glm::vec3(0.8f, 0.3f, 0.2f);
    bodies.push_back(mars);

    CelestialBody* jupiter = new CelestialBody("Jupiter", 69911, { 5.204, 0.0489, 1.304, 100.46, 273.86, 20.02, 11.86 }, 0.41, 3.13, sun);
    jupiter->color = glm::vec3(0.8f, 0.7f, 0.5f);
    bodies.push_back(jupiter);

    CelestialBody* saturn = new CelestialBody("Saturn", 58232, { 9.582, 0.0565, 2.48, 113.71, 92.43, 317.02, 29.45 }, 0.44, 26.7, sun);
    saturn->color = glm::vec3(0.9f, 0.85f, 0.5f);
    bodies.push_back(saturn);

    CelestialBody* uranus = new CelestialBody("Uranus", 25362, { 19.201, 0.0463, 0.77, 74.00, 170.96, 142.23, 84.02 }, 0.72, 97.8, sun);
    uranus->color = glm::vec3(0.5f, 0.8f, 0.9f);
    bodies.push_back(uranus);

    CelestialBody* neptune = new CelestialBody("Neptune", 24622, { 30.047, 0.0094, 1.77, 131.78, 44.97, 267.76, 164.79 }, 0.67, 28.3, sun);
    neptune->color = glm::vec3(0.2f, 0.2f, 0.8f);
    bodies.push_back(neptune);
}

void SolarSystem::initializeTextures() {
    // Wczytujemy tekstury i przypisujemy je odpowiednim obiektom
    // Zak³adamy kolejnoœæ w wektorze: 0-Sun, 3-Earth (jak wy¿ej)
    // Bezpieczniej by³oby szukaæ po nazwie, ale dla prostoty u¿ywamy indeksów lub szukania:

    for (auto body : bodies) {
        if (body->name == "Sun") {
            body->diffuseMap = loadTexture("sun.jpg");
        }
        else if (body->name == "Earth") {
            body->diffuseMap = loadTexture("earth.jpg");
            body->nightMap = loadTexture("earth_night.jpg");
        }
        else if (body->name == "Moon") {
            body->diffuseMap = loadTexture("moon.jpg");
        }
        // Tutaj mo¿esz dodaæ resztê tekstur, np:
        // else if (body->name == "Mars") body->diffuseMap = loadTexture("mars.jpg");
    }
}

void SolarSystem::update(double currentTimeDays) {
    for (auto body : bodies) {
        body->update(currentTimeDays);
    }
}

SolarSystem::~SolarSystem() {
    for (auto body : bodies) {
        delete body;
    }
    bodies.clear();
}
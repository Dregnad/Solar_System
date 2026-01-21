#include "SolarSystem.h"
#include "TextureLoader.h" 
#include <glm/glm.hpp>

SolarSystem::SolarSystem() {

    unsigned int asteroidTexture;

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

    // --- KSIÊ¯YCE MARSA ---

    // Parametry Fobosa
    CelestialBody* fobos = new CelestialBody("Fobos", 11.1, { 0.0004021, 0.0151, 1.093, 0.0, 0.0, 0.0, 0.0048 }, 0.318, 0.036, mars);
    fobos->color = glm::vec3(0.5f, 0.5f, 0.5f);
    bodies.push_back(fobos);

    // Parametry Deimosa
    CelestialBody* deimos = new CelestialBody("Deimos", 6.2, { 0.00045676, 0.0002, 0.930, 0.0, 0.0, 0.0, 0.0048 }, 1.263, 0.036, mars);
    deimos->color = glm::vec3(0.6f, 0.55f, 0.5f);
    bodies.push_back(deimos);




    CelestialBody* jupiter = new CelestialBody("Jupiter", 69911, { 5.204, 0.0489, 1.304, 100.46, 273.86, 20.02, 11.86 }, 0.41, 3.13, sun);
    jupiter->color = glm::vec3(0.8f, 0.7f, 0.5f);
    bodies.push_back(jupiter);

    // --- KSIÊ¯YCE JOWISZA ---

    // Io: 
    CelestialBody* io = new CelestialBody("Io", 1821, { 0.00950, 0.0041, 0.04, 0.0, 0.0, 0.0, 0.0048 }, 1.77, 0.0, jupiter);
    io->color = glm::vec3(1.0f, 1.0f, 0.4f); 
    bodies.push_back(io);

    // Europa:
    CelestialBody* europa = new CelestialBody("Europa", 1560, { 0.01115, 0.009, 0.47, 0.0, 0.0, 0.0, 0.0097 }, 3.55, 0.1, jupiter);
    europa->color = glm::vec3(0.9f, 0.9f, 0.8f); 
    bodies.push_back(europa);

    // Ganimedes:
    CelestialBody* ganymede = new CelestialBody("Ganymede", 2634, { 0.01383, 0.0013, 0.20, 0.0, 0.0, 0.0, 0.0196 }, 7.15, 0.3, jupiter);
    ganymede->color = glm::vec3(0.6f, 0.5f, 0.4f); 
    bodies.push_back(ganymede);

    // Kallisto: 
    CelestialBody* callisto = new CelestialBody("Callisto", 2410, { 0.01915, 0.0074, 0.20, 0.0, 0.0, 0.0, 0.0457 }, 16.69, 0.0, jupiter);
    callisto->color = glm::vec3(0.4f, 0.4f, 0.4f);
    bodies.push_back(callisto);

    CelestialBody* saturn = new CelestialBody("Saturn", 58232, { 9.582, 0.0565, 2.48, 113.71, 92.43, 317.02, 29.45 }, 0.44, 26.7, sun);
    saturn->color = glm::vec3(0.9f, 0.85f, 0.5f);
    bodies.push_back(saturn);

    // --- KSIÊ¯YCE SATURNA ---
    
    // --- TYTAN ---

    CelestialBody* titan = new CelestialBody("Titan", 2575, { 0.013991, 0.0288, 0.33, 0.0, 0.0, 0.0, 22.57 }, 15.9, 0.192, saturn);
    titan->color = glm::vec3(0.9f, 0.7f, 0.1f);
    bodies.push_back(titan);

    // --- RHEA ---

    CelestialBody* rhea = new CelestialBody("Rhea", 764, { 0.009346, 0.001, 0.33, 0.0, 0.0, 0.0, 79.8 }, 4.5, 0.192, saturn);
    rhea->color = glm::vec3(0.8f, 0.8f, 0.8f);
    bodies.push_back(rhea);

    // --- JAPET (Iapetus) ---
    CelestialBody* iapetus = new CelestialBody("Iapetus", 734, { 0.029626, 0.0286, 15.47, 0.0, 0.0, 0.0, 4.5 }, 79.3, 0.192, saturn);
    iapetus->color = glm::vec3(0.4f, 0.4f, 0.4f);
    bodies.push_back(iapetus);

    // --- DIONE ---
    CelestialBody* dione = new CelestialBody("Dione", 561, { 0.008346, 0.0022, 0.02, 0.0, 0.0, 0.0, 131.8 }, 2.7, 0.192, saturn);
    dione->color = glm::vec3(0.7f, 0.7f, 0.7f);
    bodies.push_back(dione);

    // --- TETYDA (Tethys) ---
    CelestialBody* tethys = new CelestialBody("Tethys", 531, { 0.007792, 0.0001, 1.09, 0.0, 0.0, 0.0, 190.6 }, 1.9, 0.192, saturn);
    tethys->color = glm::vec3(0.85f, 0.85f, 0.85f);
    bodies.push_back(tethys);

    CelestialBody* uranus = new CelestialBody("Uranus", 25362, { 19.201, 0.0463, 0.77, 74.00, 170.96, 142.23, 84.02 }, 0.72, 97.8, sun);
    uranus->color = glm::vec3(0.5f, 0.8f, 0.9f);
    bodies.push_back(uranus);

    // --- Ksiê¿yce Urana 

    // 1. TYTANIA (Titania)
 
    CelestialBody* titania = new CelestialBody("Titania", 788.9, { 0.0054276, 0.0011, 0.34, 0.0, 0.0, 0.0, 208.9 }, 8.7, 0.17, uranus);
    titania->color = glm::vec3(0.82f, 0.82f, 0.82f);
    bodies.push_back(titania);

    // 2. OBERON
 
    CelestialBody* oberon = new CelestialBody("Oberon", 761.4, { 0.0064143, 0.0014, 0.058, 0.0, 0.0, 0.0, 323.1 }, 13.4, 0.14, uranus);
    oberon->color = glm::vec3(0.80f, 0.78f, 0.76f);
    bodies.push_back(oberon);

    // 3. UMBRIEL
  
    CelestialBody* umbriel = new CelestialBody("Umbriel", 584.7, { 0.0042919, 0.0039, 0.205, 0.0, 0.0, 0.0, 99.5 }, 4.1, 0.10, uranus);
    umbriel->color = glm::vec3(0.45f, 0.45f, 0.45f);
    bodies.push_back(umbriel);

    // 4. ARIEL

    CelestialBody* ariel = new CelestialBody("Ariel", 578.9, { 0.0037899, 0.0012, 0.26, 0.0, 0.0, 0.0, 60.5 }, 2.5, 0.23, uranus);
    ariel->color = glm::vec3(0.88f, 0.88f, 0.90f);
    bodies.push_back(ariel);

    // 5. MIRANDA

    CelestialBody* miranda = new CelestialBody("Miranda", 235.8, { 0.0033788, 0.0013, 4.338, 0.0, 0.0, 0.0, 33.9 }, 1.4, 0.32, uranus);
    miranda->color = glm::vec3(0.92f, 0.92f, 0.92f);
    bodies.push_back(miranda);

    CelestialBody* neptune = new CelestialBody("Neptune", 24622, { 30.047, 0.0094, 1.77, 131.78, 44.97, 267.76, 164.79 }, 0.67, 28.3, sun);
    neptune->color = glm::vec3(0.2f, 0.2f, 0.8f);
    bodies.push_back(neptune);

    // --- Ksiê¿yce Neptuna 

    // 1. TRYTON (Triton)
    CelestialBody* triton = new CelestialBody("Triton", 1353, { 0.0047434, 0.000016, 156.8, 0.0, 0.0, 0.0, 5.877 }, 1.5, 0.2, neptune);
    triton->color = glm::vec3(0.9f, 0.85f, 0.8f);
    bodies.push_back(triton);

    // 2. PROTEUSZ (Proteus)
    CelestialBody* proteus = new CelestialBody("Proteus", 210, { 0.0031578, 0.0005, 0.55, 0.0, 0.0, 0.0, 1.122 }, 2.0, 0.1, neptune);
    proteus->color = glm::vec3(0.5f, 0.5f, 0.5f); 
    bodies.push_back(proteus);

    // 3. NEREIDA (Nereid)
    CelestialBody* nereid = new CelestialBody("Nereid", 170, { 0.0392265, 0.7512, 7.23, 0.0, 0.0, 0.0, 360.13 }, 2.0, 0.1, neptune);
    nereid->color = glm::vec3(0.7f, 0.7f, 0.7f); 
    bodies.push_back(nereid);


    // PLUTON 
    CelestialBody* pluto = new CelestialBody("Pluto", 1188, { 39.5886, 0.2518, 17.15, 110.2924, 113.7090, 38.6837, 247.94 }, 0.0022, 122.5, sun);
    pluto->color = glm::vec3(0.7f, 0.63f, 0.55f);
    bodies.push_back(pluto);




    // --- PAS ASTEROID  ---
    int numberOfAsteroids = 100; 

    for (int i = 0; i < numberOfAsteroids; ++i) {

        float a = 2.2f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (3.4f - 2.2f)));

        // 2. Losowanie parametrów orbitalnych
        float e = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.05f));
        float i_angle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 5.0f));
        float lan = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 360.0f));
        float arg = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 360.0f));
        float m = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 360.0f));

        // 3. Okres orbitalny 
        float period = sqrt(pow(a, 3));



        float radius = 50.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 400.0f));

        std::string name = "Asteroid_" + std::to_string(i);
        CelestialBody* asteroid = new CelestialBody(name, radius, { a, e, i_angle, lan, arg, m, period }, 1.0, 0.0, sun);

        asteroid->color = glm::vec3(0.5f, 0.45f, 0.4f);

        bodies.push_back(asteroid);
    }



    CelestialBody* halley = new CelestialBody("Comet_Halley", 60.0, { 17.834, 0.9671, 162.26, 58.42, 111.33, 38.38, 75.32 }, 1.0, 0.0, sun);
    halley->color = glm::vec3(0.8f, 0.9f, 1.0f);
    bodies.push_back(halley);

    // 2. Kometa Enckego
 
    CelestialBody* encke = new CelestialBody("Comet_Encke", 40.0, { 2.214, 0.848, 11.78, 334.56, 186.54, 0.0, 3.30 }, 1.0, 0.0, sun);
    encke->color = glm::vec3(0.7f, 0.8f, 0.9f);
    bodies.push_back(encke);

    

    // 4. Kometa Borrelly’ego
    CelestialBody* borrelly = new CelestialBody("Comet_Borrelly", 50.0, { 3.59, 0.624, 30.3, 75.4, 353.4, 0.0, 6.8 }, 1.0, 0.0, sun);
    borrelly->color = glm::vec3(0.6f, 0.7f, 0.8f);
    bodies.push_back(borrelly);



}

void SolarSystem::initializeTextures() {
    unsigned int asteroidTex = loadTexture("textures/asteroid.jpg");
    unsigned int cometTex = loadTexture("textures/comet.jpg");
    for (auto body : bodies) {
        if (body->name.find("Asteroid") != std::string::npos) {
            body->diffuseMap = asteroidTex;
            continue;
        }
        if (body->name.find("Comet") != std::string::npos) {
            body->diffuseMap = loadTexture("textures/comet.jpg"); 
            continue;
        }

        if (body->name == "Sun") {
            body->diffuseMap = loadTexture("sun.jpg");
        }
        else if (body->name == "Mercury") {
            body->diffuseMap = loadTexture("mercury.jpg");
        }
        else if (body->name == "Venus") {
            body->diffuseMap = loadTexture("venus.jpg");
        }
        else if (body->name == "Earth") {
            body->diffuseMap = loadTexture("earth.jpg");
            body->nightMap = loadTexture("earth_night.jpg");
        }
        else if (body->name == "Moon") {
            body->diffuseMap = loadTexture("moon.jpg");
        }
        else if (body->name == "Mars") {
            body->diffuseMap = loadTexture("mars.jpg");
        }
       
        else if (body->name == "Jupiter") {
            body->diffuseMap = loadTexture("jupiter.jpg");
        }
        
        else if (body->name == "Saturn") {
            body->diffuseMap = loadTexture("saturn.jpg");
            body->ringMap = loadTexture("saturn_ring.png");
        }
        else if (body->name == "Uranus") {
            body->diffuseMap = loadTexture("uranus.jpg");
        }
        else if (body->name == "Neptune") {
            body->diffuseMap = loadTexture("neptune.jpg");
        }
        else if (body->name == "Pluto") {
            body->diffuseMap = loadTexture("plutomap2k.jpg");
        }
        else if (body->name == "Io") {
            body->diffuseMap = loadTexture("textures/io.jpg");
        }
        else if (body->name == "Europa") {
            body->diffuseMap = loadTexture("textures/europa.jpg");
        }
        else if (body->name == "Ganymede") {
            body->diffuseMap = loadTexture("textures/ganymede.jpg");
        }
        else if (body->name == "Callisto") {
            body->diffuseMap = loadTexture("textures/callisto.jpg");
        }
        else if (body->name == "Titan") {
            body->diffuseMap = loadTexture("textures/titan.jpg");
        }
        else if (body->name == "Rhea") {
            body->diffuseMap = loadTexture("textures/rhea.jpg");
        }
        else if (body->name == "Iapetus") {
            body->diffuseMap = loadTexture("textures/iapetus.jpg");
        }
        else if (body->name == "Dione") {
            body->diffuseMap = loadTexture("textures/dione.jpg");
        }
        else if (body->name == "Tethys") {
            body->diffuseMap = loadTexture("textures/tethys.jpg");
        }
        else if (body->name == "Titania") {
            body->diffuseMap = loadTexture("textures/titania.jpg");
        }
        else if (body->name == "Oberon") {
            body->diffuseMap = loadTexture("textures/oberon.jpg");
        }
        else if (body->name == "Umbriel") {
            body->diffuseMap = loadTexture("textures/umbriel.jpg");
        }
        else if (body->name == "Ariel") {
            body->diffuseMap = loadTexture("textures/ariel.jpg");
        }
        else if (body->name == "Miranda") {
            body->diffuseMap = loadTexture("textures/miranda.jpg");
        }
        else if (body->name == "Triton") {
            body->diffuseMap = loadTexture("textures/triton.jpg");
        }
        else if (body->name == "Proteus") {
            body->diffuseMap = loadTexture("textures/proteus.jpg");
        }
        else if (body->name == "Nereid") {
            body->diffuseMap = loadTexture("textures/nereid.jpg");
        }
        
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
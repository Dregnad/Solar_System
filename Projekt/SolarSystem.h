#pragma once
#include <vector>
#include <string>
#include "Body.h"

class SolarSystem {
public:
    std::vector<CelestialBody*> bodies;

    SolarSystem();  // Konstruktor utworzy planety
    ~SolarSystem(); // Destruktor posprz¹ta pamiêæ

    void initializeTextures(); // Za³aduje tekstury
    void update(double currentTimeDays); // Zaktualizuje pozycje
};
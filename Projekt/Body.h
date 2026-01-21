#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <cmath>

// Simulation Constants
extern double deltaTimeDouble;
extern double simulationSpeed;

const double PI = 3.14159265358979323846;
const double AU_SCALE = 400.0; // Scale distance to make solar system visible

struct OrbitalElements {
    double semiMajorAxis;       // in AU
    double eccentricity;
    double inclination;         // in Degrees
    double ascendingNode;       // in Degrees
    double periapsis;           // in Degrees
    double meanAnomalyEpoch;    // in Degrees
    double orbitalPeriod;       // in Years
};

class CelestialBody {
public:
    std::string name;
    OrbitalElements orbit;
    double radius;              // in km
    double rotationPeriod;      // in Days
    double axialTilt;           // in Degrees

    CelestialBody* parent;
    std::vector<CelestialBody*> children;

    glm::dvec3 position;        // Position relative to parent
    glm::dvec3 worldPosition;   // Absolute position in world space

    double currentRotationAngle;
    glm::vec3 color;

    // Textures
    unsigned int diffuseMap = 0;
    unsigned int nightMap = 0;
    unsigned int ringMap = 0;

    // Orbit Rendering
    unsigned int VAO_Orbit = 0;
    std::vector<glm::vec3> orbitPath;

  
    CelestialBody() {
        position = glm::dvec3(0.0);
        worldPosition = glm::dvec3(0.0);
        color = glm::vec3(0.5f); 
        radius = 1.0;
        rotationPeriod = 0.0;
        axialTilt = 0.0;
        currentRotationAngle = 0.0;

        name = "Unknown";
        parent = nullptr;

        // Wyzerowanie parametrów orbity
        orbit = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };

        diffuseMap = 0;
        nightMap = 0;
        ringMap = 0;
        VAO_Orbit = 0;
    }
   

    CelestialBody(std::string n, double r, OrbitalElements o, double rotP, double tilt, CelestialBody* p = nullptr)
        : name(n), radius(r), orbit(o), rotationPeriod(rotP), axialTilt(tilt), parent(p), currentRotationAngle(0.0)
    {
        diffuseMap = 0;
        nightMap = 0;
        ringMap = 0;
        color = glm::vec3(1.0f, 1.0f, 1.0f); 

       
        position = glm::dvec3(0.0);
        worldPosition = glm::dvec3(0.0);

        if (parent) {
            parent->children.push_back(this);
          
            generateFullOrbit(500);
        }
    }

  
    double solveKepler(double M, double e) {
        double E = M;
        for (int i = 0; i < 30; i++) {
            double delta = E - e * sin(E) - M;
            E = E - delta / (1.0 - e * cos(E));
            if (std::abs(delta) < 1e-6) break;
        }
        return E;
    }

    
    glm::dvec3 calculatePosition(double M) {
        double E = solveKepler(M, orbit.eccentricity);

       
        double x_orb = orbit.semiMajorAxis * (cos(E) - orbit.eccentricity);
        double y_orb = orbit.semiMajorAxis * sqrt(1.0 - orbit.eccentricity * orbit.eccentricity) * sin(E);

      
        double i = glm::radians(orbit.inclination);
        double om = glm::radians(orbit.ascendingNode);
        double w = glm::radians(orbit.periapsis);

        double x1 = x_orb * cos(w) - y_orb * sin(w);
        double y1 = x_orb * sin(w) + y_orb * cos(w);

        double x2 = x1;
        double y2 = y1 * cos(i);
        double z2 = -y1 * sin(i);

        double x3 = x2 * cos(om) - y2 * sin(om);
        double y3 = x2 * sin(om) + y2 * cos(om);
        double z3 = z2;

       
        return glm::dvec3(x3, z3, -y3) * AU_SCALE;
    }

    
    void generateFullOrbit(int segments) {
        orbitPath.clear();
        for (int i = 0; i <= segments; i++) {
            double M = (2.0 * PI * i) / segments;
            glm::dvec3 pos = calculatePosition(M);
            orbitPath.push_back(glm::vec3(pos));
        }
    }

    void update(double timeDays) {
        if (parent == nullptr) {
     

            if (name == "Sun") {
                position = glm::dvec3(0.0);
                worldPosition = position;
            }
           
        }
        else {
           
            double n = (2.0 * PI) / (orbit.orbitalPeriod * 365.25);
            double M = glm::radians(orbit.meanAnomalyEpoch) + n * timeDays;

          
            position = calculatePosition(M);

           
            worldPosition = parent->worldPosition + position;
        }

      
        if (rotationPeriod > 0.0) {
            double angleChange = (360.0 / rotationPeriod) * simulationSpeed * deltaTimeDouble;
            currentRotationAngle = fmod(currentRotationAngle + angleChange, 360.0);
        }
    }
};
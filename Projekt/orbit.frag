#version 330 core
out vec4 FragColor;

// Tutaj odbieramy kolor z main.cpp
uniform vec3 orbitColor;

void main()
{
    // Ustawiamy kolor pixela na ten odebrany, z pe³n¹ nieprzezroczystoœci¹ (1.0)
    FragColor = vec4(orbitColor, 1.0); 
}
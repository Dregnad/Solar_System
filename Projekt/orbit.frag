#version 330 core
out vec4 FragColor;

// Zmiana z vec3 na vec4, aby przyj¹æ te¿ kana³ Alpha (przezroczystoœæ)
uniform vec4 orbitColor;

void main()
{
    // Przypisujemy kolor bezpoœrednio, bo Alpha jest ju¿ w orbitColor
    FragColor = orbitColor;
}
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D ringTexture;
uniform vec3 lightPos; // S³oñce

void main()
{
    // Pobieramy kolor z tekstury. Tekstura musi mieæ "paski" id¹ce wzd³u¿ osi V (pionowo)
    // Poniewa¿ w meshu u¿yliœmy V jako promienia (0=wewn¹trz, 1=zewn¹trz)
    vec4 texColor = texture(ringTexture, vec2(TexCoords.y, 0.5)); 

    // Jeœli pixel jest przezroczysty, odrzucamy go (dla ostrych krawêdzi)
    if(texColor.a < 0.1)
        discard;

    // Proste oœwietlenie, ¿eby pierœcieñ ciemnia³ w cieniu planety (opcjonalne)
    // Tutaj dajemy sta³¹ jasnoœæ, bo pierœcienie s¹ jasne
    FragColor = texColor;
}
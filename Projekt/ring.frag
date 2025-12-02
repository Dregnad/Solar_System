#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D ringTexture;
uniform vec3 lightPos; 

void main()
{
    // Use TexCoords.y for radial lookup (y=0 is inner, y=1 is outer)
    // We use 0.5 for the X coordinate assuming the texture is a vertical gradient strip
    vec4 texColor = texture(ringTexture, vec2(TexCoords.y, 0.5));
    
    // Discard transparent parts (black/alpha 0)
    if(texColor.a < 0.1)
        discard;

    FragColor = texColor;
}
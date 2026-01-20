#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D ringTexture;
uniform vec3 lightPos; 

void main()
{
    vec4 texColor = texture(ringTexture, vec2(TexCoords.y, 0.5));
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}
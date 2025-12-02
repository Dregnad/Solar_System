#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Uniformy
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isSun; // Czy to obiekt emituj¹cy œwiat³o (S³oñce lub T³o)

// Tekstury
uniform sampler2D diffuseTexture; 
uniform sampler2D nightTexture;   
uniform bool hasTexture;          
uniform bool hasNightTexture;     

void main()
{
    // 1. Kolor bazowy
    vec4 texColor = vec4(objectColor, 1.0);
    if (hasTexture) {
        texColor = texture(diffuseTexture, TexCoords);
    }

    // --- S£OÑCE I T£O (SKYBOX) ---
    if(isSun) {
        FragColor = texColor; 
    } 
    // --- PLANETY ---
    else {
        // Œwiat³o otoczenia (Ambient)
        vec3 ambient = 0.05 * texColor.rgb;

        // Œwiat³o rozproszone (Diffuse)
        vec3 normal = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * texColor.rgb;

        // Odb³ysk (Specular)
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        vec3 specular = vec3(0.5) * spec; 

        vec3 result = ambient + diffuse + specular;

        // --- ZIEMIA W NOCY (ŒWIAT£A MIAST) ---
        if (hasNightTexture) {
            vec3 nightColor = texture(nightTexture, TexCoords).rgb;
            
            // dayFactor: 1.0 w dzieñ, 0.0 w nocy. 
            // U¿ywamy smoothstep dla p³ynnego przejœcia na granicy cienia.
            float dayFactor = smoothstep(-0.25, 0.25, dot(normal, lightDir));
            
            // Mieszamy: Œwiat³o dzienne vs Œwiat³a nocne
            result = mix(nightColor, result, dayFactor);
        }

        FragColor = vec4(result, 1.0);
    }
}
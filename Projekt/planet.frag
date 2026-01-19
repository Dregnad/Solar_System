#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool isSun;
uniform bool isCloudLayer;
uniform bool hasAtmosphere;
uniform vec3 atmosphereColorRGB;

uniform float time;
uniform float fov;

uniform sampler2D diffuseTexture;
uniform sampler2D detailTexture;
uniform sampler2D nightTexture;
uniform bool hasTexture;
uniform bool hasNightTexture;

// --- ECLIPSE: NOWY UNIFORM ---
uniform float eclipseFactor; // 1.0 = S³oñce widoczne, 0.0 = S³oñce zas³oniête

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main()
{
    // --- 1. RENDEROWANIE S£OÑCA I T£A ---
    if(isSun) {
        vec4 texColor = vec4(objectColor, 1.0);
        if (hasTexture) texColor = texture(diffuseTexture, TexCoords);

        float distFromCenter = length(FragPos);

        // A. SKYBOX (T£O)
        if (distFromCenter > 2000.0) {
            float zoomFactor = 1.0 - smoothstep(10.0, 45.0, fov);
            vec3 baseColor = texture(diffuseTexture, TexCoords).rgb;
            vec3 detailColor = texture(detailTexture, TexCoords * 10.0).rgb;
            
            vec3 mixedSky = baseColor + (detailColor * zoomFactor * 0.5);

            float noise = random(TexCoords);
            float flash = sin(time * 2.0 + noise * 100.0);
            flash = 0.7 + 0.3 * flash;
            float brightness = dot(mixedSky, vec3(0.299, 0.587, 0.114));
            vec3 finalSky = mixedSky * mix(1.0, flash, brightness);
            finalSky += vec3(brightness * 0.5); 
            finalSky = pow(finalSky, vec3(1.2)); 
            
            // --- ECLIPSE: Przyciemnianie gwiazd podczas zaæmienia (opcjonalne) ---
            // Gwiazdy s¹ widoczne lepiej gdy S³oñce jest zas³oniête, 
            // ale tutaj symulujemy "oko", wiêc jeœli S³oñce znika, reszta nie musi ciemnieæ.
            // Zostawiamy bez zmian lub mno¿ymy przez eclipseFactor dla efektu mroku.
            
            FragColor = vec4(finalSky, 1.0);
            return;
        }
        
        // B. S£OÑCE (KULA)
        else {
             float distToCam = length(viewPos - FragPos);
             float proximity = 1.0 - smoothstep(1000.0, 15000.0, distToCam);
             proximity = pow(proximity, 2.0); 

             vec3 baseSun = texColor.rgb;
             baseSun = mix(baseSun, vec3(1.0, 0.95, 0.8), proximity * 0.3); 
             baseSun *= (1.0 + proximity * 0.5);

             vec3 norm = normalize(Normal);
             vec3 viewDir = normalize(viewPos - FragPos);
             float fresnel = 1.0 - max(dot(norm, viewDir), 0.0);
             float glowWidth = mix(4.5, 2.0, proximity);
             float rimFactor = pow(fresnel, glowWidth);

             vec3 orangeGlow = vec3(1.0, 0.6, 0.0);
             float glowIntensity = 2.0 + proximity * 2.5;

            vec3 finalSunColor = baseSun + orangeGlow * rimFactor * glowIntensity;

             // --- POPRAWKA REALIZMU ---
             // eclipseFactor wp³ywa mocniej na œrodek s³oñca (baseSun), a s³abiej na koronê (orangeGlow).
             // Dziêki temu przy zaæmieniu widaæ piêkn¹ obwódkê wokó³ czarnej planety.
             
             
             
             // 2. Dodajemy resztkow¹ koronê, jeœli jesteœmy w fazie pó³cienia (eclipseFactor < 0.5)
             // To sprawia, ¿e krawêdŸ s³oñca "œwieci" zza planety.
             float coronaResidual = (1.0 - eclipseFactor) * 0.3 * rimFactor;
             finalSunColor += vec3(1.0, 0.5, 0.2) * coronaResidual;

             FragColor = vec4(finalSunColor, 1.0);
             return;
        }
    }

    // --- 2. CHMURY ---
    if (isCloudLayer) {
        vec4 cloudData = texture(diffuseTexture, TexCoords);
        float alpha = cloudData.r; 
        if(alpha < 0.1) discard;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        
        // --- ECLIPSE: Chmury ciemniej¹ przy zaæmieniu ---
        diff *= eclipseFactor;

        vec3 cloudColor = vec3(1.0, 1.0, 1.0) * (diff * 0.9 + 0.1);
        FragColor = vec4(cloudColor, alpha * 0.8);
        return;
    }

    // --- 3. PLANETY ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    
    // --- ECLIPSE: Powierzchnia planety ciemnieje (jeœli cieñ pada na inne obiekty) ---
    // W tym prostym modelu zaæmienie dotyczy kamery, wiêc to wp³ywa g³ównie na S³oñce,
    // ale mo¿emy te¿ przyciemniæ oœwietlenie ambientowe.
    diff *= eclipseFactor; 

    vec3 dayColor = objectColor;
    if (hasTexture) {
        dayColor = texture(diffuseTexture, TexCoords).rgb;
    }

    vec3 finalColor = vec3(0.0);

    if (hasNightTexture) {
        vec3 nightColor = texture(nightTexture, TexCoords).rgb;
        float mixFactor = smoothstep(-0.1, 0.1, dot(norm, lightDir));
        finalColor = mix(nightColor * 1.5, dayColor * diff, mixFactor);
    } 
    else {
        vec3 ambient = 0.02 * dayColor;
        finalColor = ambient + (dayColor * diff);
    }

    if (diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        vec3 specular = vec3(0.3) * spec * eclipseFactor; // Specular te¿ znika
        finalColor += specular;
    }

    if (hasAtmosphere) {
        float fresnel = 1.0 - max(dot(norm, viewDir), 0.0);
        fresnel = pow(fresnel, 3.0); 
        // Atmosfera œwieci s³abiej przy zaæmieniu
        vec3 atmosphereEffect = atmosphereColorRGB * fresnel * clamp(diff + 0.2, 0.0, 1.0) * eclipseFactor;
        finalColor += atmosphereEffect;
    }

    FragColor = vec4(finalColor, 1.0);
}
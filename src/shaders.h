#include <glm/glm.hpp>
#include "uniforms.h"
#include "fragment.h"
#include <SDL.h>
#include <string>
#include "FastNoiseLite.h"
#include "settings.h"

//SUN
FastNoiseLite sunNoise;

Color getSunNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 darkColor = glm::vec3(0.73f, 0.06f, 0.03f);
    glm::vec3 brightColor = glm::vec3(0.99f, 0.45f, 0.04f);

    float noise;

    sunNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    float animationSpeed = 0.01f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    sunNoise.SetFrequency(0.02 + (sin(3 * time) + 1) * 0.01);
    int zoom = 600;
    noise = (1 + sunNoise.GetNoise(x * zoom, y * zoom, z * zoom)) / 2.5f;

    // Occasional white flash
    float flashProbability = 0.002; // Adjust the probability of a flash
    if (rand() / static_cast<float>(RAND_MAX) < flashProbability) {
        fragmentColor = Color(1.0f, 1.0f, 1.0f); // White flash
    } else {
        // Simulate eruptions or explosions
        float eruptionProbability = 0.001; // Adjust the probability of an eruption
        if (rand() / static_cast<float>(RAND_MAX) < eruptionProbability) {
            fragmentColor = Color(1.0f, 0.5f, 0.0f); // Orange eruption color
        } else {
            glm::vec3 sunNoiseColor = mix(brightColor, darkColor, noise * 2.0f);
            fragmentColor = Color(sunNoiseColor.x, sunNoiseColor.y, sunNoiseColor.z);
        }
    }

    return fragmentColor;
}


//EARTH
FastNoiseLite worldNoise;
FastNoiseLite surfaceNoise;
FastNoiseLite atmosphereNoise;

struct WorldColors {
    glm::vec3 landColor = {35.0f / 255, 70.0f / 255, 25.0f / 255};
    glm::vec3 surfaceColor = {70.0f / 255, 100.0f / 255, 45.0f / 255};
    glm::vec3 shoreColor = {250.0f / 255, 220.0f / 255, 100.0f / 255};
    glm::vec3 oceanColor = {30.0f / 255, 40.0f / 255, 90.0f / 255};
    glm::vec3 seaColor = {80.0f / 255, 110.0f / 255, 190.0f / 255};
    glm::vec3 skyColor = {240.0f / 255, 240.0f / 255, 255.0f / 255};
};

void setWorldNoiseParameters() {
    worldNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    worldNoise.SetFrequency(0.002);
    worldNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    worldNoise.SetFractalOctaves(6);

    surfaceNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    surfaceNoise.SetFractalOctaves(7);
    surfaceNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    surfaceNoise.SetFrequency(0.005);

    atmosphereNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    atmosphereNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    float animationSpeed = 0.01f;
    atmosphereNoise.SetFractalOctaves(6);
}

Color getWorldNoise(float x, float y, float z) {
    Color fragmentColor;

    WorldColors colors;
    setWorldNoiseParameters();

    float offsetX = 70.0f;
    float offsetY = 70.0f;
    float zoom = 500.0f;

    float firstLayer = worldNoise.GetNoise((x + offsetX) * zoom, (y + offsetY) * zoom, z * zoom);
    glm::vec3 tempColor;

    if (firstLayer < 0.2) {
        zoom = 800.0f;
        float continentNoise = surfaceNoise.GetNoise(x * zoom, y * zoom, z * zoom);

        // Adjust the colors for land and surface
        tempColor = colors.landColor + (colors.surfaceColor * continentNoise);
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};

        // Add some variation to shore color
        if (firstLayer > 0.18) {
            tempColor = colors.shoreColor + colors.surfaceColor * continentNoise;
            fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
        }
    } else {
        float oceanNoise = surfaceNoise.GetNoise(x * zoom, y * zoom, z * zoom);

        // Adjust the colors for ocean and sea
        tempColor = colors.oceanColor + colors.seaColor * oceanNoise;
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
    }

    zoom = 100.0f;

    // Add more dynamics to cloud color
    float cloudNoise = atmosphereNoise.GetNoise((x + frame * 0.007f) * zoom, y * zoom, z * zoom);
    if (cloudNoise > 0.2) {
        tempColor = tempColor + colors.skyColor * cloudNoise;
        fragmentColor = {tempColor.x, tempColor.y, tempColor.z};
    }

    return fragmentColor;
}


//MOON
FastNoiseLite moonNoise;

Color getMoonNoise(float x, float y, float z) {

    Color fragmentColor;

    glm::vec3 darkColor = {95.0f/255, 106.0f/255, 116.0f/255};
    glm::vec3 lightColor = {209.0f/255, 195.0f/255, 183.0f/255};

    moonNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    moonNoise.SetFrequency(0.2);
    moonNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 60.0f;

    float noise;

    noise = moonNoise.GetNoise(x*zoom, y*zoom, z*zoom);

    glm::vec3 moonNoiseColor = mix(lightColor, darkColor, noise);

    fragmentColor = Color(moonNoiseColor.x, moonNoiseColor.y, moonNoiseColor.z);

    return fragmentColor;
}

//BLUEY
FastNoiseLite blueyNoise;

Color getBlueyNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 baseColor = {0.0f, 0.0f, 1.0f};  // Fully blue base color
    glm::vec3 stripeColor = {1.0f, 1.0f, 1.0f};  // White color for lightning stripes

    blueyNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    blueyNoise.SetFrequency(0.2);
    blueyNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 40.0f;

    float noise = blueyNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Introduce lightning stripe pattern
    float stripeFactor = abs(sin(x * 10.0) * cos(y * 10.0) * sin(z * 10.0));

    // Mix base color with stripe color based on the stripe factor
    glm::vec3 finalColor = mix(baseColor, stripeColor, stripeFactor);

    fragmentColor = Color(finalColor.x, finalColor.y, finalColor.z);

    return fragmentColor;
}

//DISCOBALL
FastNoiseLite discoballNoise;

Color getDiscoBallPlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    Color trippyColor = {73, 218, 37};
    float noise;

    discoballNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    float animationSpeed = 0.001f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    discoballNoise.SetFrequency(0.02 + sin(3 * time) * 0.01);
    int zoom = 200;

    // Calculate distance from the center
    float distanceToCenter = sqrt(x * x + y * y + z * z);

    // Create a giant circle in the middle with changing colors
    float circleRadius = 0.3;
    float circleAnimationSpeed = 0.01;
    float circleColorFactor = sin(time * circleAnimationSpeed) * 0.5 + 0.5;
    Color circleColor = {static_cast<float>(1.0 - circleColorFactor), circleColorFactor, 0.0};

    // Check if the point is inside the circle
    if (distanceToCenter < circleRadius) {
        noise = 1.0;  // Full intensity inside the circle
        fragmentColor = circleColor * noise * 15.0f;
    } else {
        // Surround the circle with rotating rainbow colors
        float angle = atan2(y, x);
        angle = (angle < 0) ? angle + 2 * M_PI : angle;

        // Introduce rotation factor based on time
        float rotationSpeed = 0.5;  // Adjust the rotation speed
        angle += time * rotationSpeed;

        float rainbowFactor = fmod(angle, M_PI) / M_PI;  // Map angle to [0, 1]
        fragmentColor = trippyColor * rainbowFactor * 15.0f;
    }

    return fragmentColor;
}

//GAS PLANET
FastNoiseLite gasPlanetNoise;

Color getGasPlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 primaryColor = {0.2f, 0.5f, 0.8f};    // Blue-ish color
    glm::vec3 secondaryColor = {1.0f, 0.8f, 0.2f};  // Orange-ish color

    gasPlanetNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    gasPlanetNoise.SetFrequency(0.2);
    gasPlanetNoise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
    gasPlanetNoise.SetFractalOctaves(8);
    gasPlanetNoise.SetFractalWeightedStrength(0.2);
    gasPlanetNoise.SetDomainWarpAmp(7.0);
    gasPlanetNoise.DomainWarp(x, y, z);

    float zoom = 20.0f;

    float noise;

    noise = gasPlanetNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Use a modified pattern with the sum of primary and secondary colors
    glm::vec3 modifiedColor = primaryColor * (1.0f - noise) + secondaryColor * noise;

    // Clamp the color values to the [0, 1] range
    modifiedColor = glm::clamp(modifiedColor, 0.0f, 1.0f);

    fragmentColor = Color(modifiedColor.x, modifiedColor.y, modifiedColor.z);

    return fragmentColor;
}

//GREENY PLANET
FastNoiseLite greenyNoise;

Color getGreenyPlanetNoise(float x, float y, float z) {
    Color fragmentColor;

    // Make the entire texture green
    glm::vec3 greenColor = {0.0f, 1.0f, 0.0f};

    // Trippy color for extra effects
    glm::vec3 trippyColor = {1.0f, 0.0f, 1.0f};

    greenyNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    greenyNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    greenyNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);

    float animationSpeed = 0.01f; // Adjust the speed of the animation
    float time = frame * animationSpeed;
    float frequency = 0.05 + std::sin(time) * 0.01;  // Modulate frequency with a sine function

    greenyNoise.SetFrequency(frequency);

    int zoom = 400;

    float noise = greenyNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    if (noise < 0) {
        noise = noise * 0.2f;  // Adjust the strength of the trippy effect
    }

    // Blend green color with trippy effects
    glm::vec3 tmpColor = greenColor + trippyColor * noise;

    // Clamp the color values to the [0, 1] range
    tmpColor = glm::clamp(tmpColor, 0.0f, 1.0f);

    fragmentColor = {tmpColor.x, tmpColor.y, tmpColor.z};

    return fragmentColor;
}

//MAGNETO PLANET
FastNoiseLite magnetoNoise;

Color getMagnetoNoise(float x, float y, float z) {
    Color fragmentColor;

    glm::vec3 redColor = {1.0f, 0.0f, 0.0f};     // Red color
    glm::vec3 purpleColor = {0.5f, 0.0f, 0.5f};  // Purple color
    glm::vec3 blackColor = {0.0f, 0.0f, 0.0f};   // Black color
    glm::vec3 whiteColor = {1.0f, 1.0f, 1.0f};   // White color

    magnetoNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    magnetoNoise.SetFrequency(0.2);
    magnetoNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    float zoom = 40.0f;

    float noise = magnetoNoise.GetNoise(x * zoom, y * zoom, z * zoom);

    // Determine the color based on noise value
    glm::vec3 colorMix = (noise > 0.0) ? mix(redColor, blackColor, abs(noise)) : mix(purpleColor, whiteColor, abs(noise));

    fragmentColor = Color(colorMix.x, colorMix.y, colorMix.z);

    return fragmentColor;
}

Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    // Aplicar las transformaciones al vértice utilizando las matrices de uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

    // Perspectiva
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Aplicar transformación del viewport
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);
    
    // Transformar la normal
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    return Vertex{
        glm::vec3(screenVertex),
        transformedNormal,
        vertex.position,
        clipSpaceVertex.w > 4.0f,
    };
}

Fragment fragmentShader(Fragment& fragment, const std::string name) {

    float x = fragment.originalPos.x;
    float y = fragment.originalPos.y;
    float z = fragment.originalPos.z;

    if (name == "earth") {
        
        fragment.color = getWorldNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "moon") {

        fragment.color = getMoonNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "sun") {
        fragment.color = getSunNoise(x,y,z);
    }

    else if (name == "greeny") {
        fragment.color = getGreenyPlanetNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "discoball") {
        fragment.color = getDiscoBallPlanetNoise(x,y,z) * 1.0f;
    }

    else if (name == "gas") {
        fragment.color = getGasPlanetNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "bluey") {
        fragment.color = getBlueyNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "magneto") {
        fragment.color = getMagnetoNoise(x,y,z) * fragment.intensity;
    }

    else if (name == "ship") {
        fragment.color = fragment.color * fragment.intensity;
    }

    return fragment;
}

#include <iostream>
#include <string>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Color.h"
#include "loadModel.h"
#include "framebuffer.h"
#include "uniforms.h"
#include "shaders.h"
#include "triangle.h"
#include "camera.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Color currentColor;

const std::string planetPath = "../models/sphere.obj";
const std::string navePath = "../models/Laboratorio3.obj";
Color clearColor(0, 0, 0);  // Color del fondo

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<Face> faces;
std::vector<Vertex> verticesArray;

std::vector<glm::vec3> naveVertices;
std::vector<glm::vec3> naveNormals;
std::vector<Face> naveFaces;
std::vector<Vertex> naveVerticesArray;

Uniforms uniforms;

glm::mat4 model = glm::mat4(1);
glm::mat4 view = glm::mat4(1);
glm::mat4 projection = glm::mat4(1);

enum class Primitive {
    TRIANGLES,
};

struct Nave {
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    float rotationAngle;
    float movSpeed;
    float rotSpeed;
};

struct Planet {
    float rotAngle;
    float transAngle;
    float transRadius;
    float speed;
    glm::vec3 axis;
    glm::vec3 worldPos;
    glm::vec3 scaleFactor;
    std::string planet;
    std::vector<Planet> satellites;
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: No se puedo inicializar SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Proyecto 1: Space Travel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error: No se pudo crear una ventana SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error: No se pudo crear SDL_Renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

std::vector<std::vector<Vertex>> primitiveAssembly(
    Primitive polygon,
    const std::vector<Vertex>& transformedVertices
) {
    std::vector<std::vector<Vertex>> assembledVertices;

    switch (polygon) {
        case Primitive::TRIANGLES: {
            assert(transformedVertices.size() % 3 == 0 && "El número de vértices debe ser un múltiplo de 3 para triángulos.");

            for (size_t i = 0; i < transformedVertices.size(); i += 3) {
                std::vector<Vertex> triangle = {
                    transformedVertices[i],
                    transformedVertices[i + 1],
                    transformedVertices[i + 2]
                };
                assembledVertices.push_back(triangle);
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo." << std::endl;
            break;
    }

    return assembledVertices;
}

std::vector<Fragment> rasterize(Primitive primitive, const std::vector<std::vector<Vertex>>& assembledVertices, std::string name, glm::vec3 worldPos) {
    std::vector<Fragment> fragments;

    switch (primitive) {
        case Primitive::TRIANGLES: {
            for (const std::vector<Vertex>& triangleVertices : assembledVertices) {
                assert(triangleVertices.size() == 3 && "Triangle vertices must contain exactly 3 vertices.");
                std::vector<Fragment> triangleFragments = triangle(triangleVertices[0], triangleVertices[1], triangleVertices[2], name, worldPos);
                fragments.insert(fragments.end(), triangleFragments.begin(), triangleFragments.end());
            }
            break;
        }
        default:
            std::cerr << "Error: No se reconoce el tipo primitivo para rasterización." << std::endl;
            break;
    }

    return fragments;
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 45.0f;
    float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight) {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

void setUpRender(Planet& model) {
    float rotationAngle = model.rotAngle;
    float translationAngle = model.transAngle;

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);

    if (model.planet != "sun") {
        model.rotAngle += 1;
        model.transAngle += model.speed;
    }
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis);

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), model.scaleFactor);

    model.worldPos.x = model.axis.x + (model.transRadius * cos(glm::radians(translationAngle)));
    model.worldPos.z = model.axis.z + (model.transRadius * sin(glm::radians(translationAngle)));

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), model.worldPos);

    // Calcular la matriz de modelo
    uniforms.model = translation * rotation * scale;
}

void setUpOrbit(Planet& planet) {
    // Agregar puntos para 360 grados
    std::vector<Vertex> orbitsVertices;
    for (float i = 0.0f; i < 360.0f; i += 1.0f)
    {

        Vertex vertex = {glm::vec3(planet.axis.x + planet.transRadius * cos(glm::radians(i)),
                        0.0f,
                                   planet.axis.z + planet.transRadius * sin(glm::radians(i))),
                        glm::vec3(1.0f)};
        Vertex transformedVertex = vertexShader(vertex, uniforms);
        orbitsVertices.push_back(transformedVertex);
    }

    for (Vertex vert : orbitsVertices)
    {

        if (vert.position.x < 0 || vert.position.y < 0  ||  vert.position.y > SCREEN_HEIGHT || vert.position.x > SCREEN_WIDTH)
            continue;

        Fragment fragment = {
            vert.position,
            Color(255, 255, 255),
            vert.position.z,
            1.0f,
            vert.position};
        point(fragment);
    }
}

void render(Primitive polygon, std::string name, std::vector<Vertex>& modelVertices, glm::vec3 worldPos){

    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices;

    for (const Vertex& vertex : modelVertices) {
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }

    // 2. Primitive Assembly
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(polygon, transformedVertices);

    // 3. Rasterization
    std::vector<Fragment> fragments = rasterize(polygon, assembledVertices, name, worldPos);

    // 4. Fragment Shader
    for (Fragment& fragment : fragments) {
        // Apply the fragment shader to compute the final color
        fragment = fragmentShader(fragment, name);
        point(fragment);
    }

}

void fastTravel(SDL_Renderer* renderer, const Camera& camera) {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    Uint32 startTime = SDL_GetTicks();
    const Uint32 animationDuration = 750;

    while (true) {
        Uint32 currentTime = SDL_GetTicks();

        Uint32 elapsedTime = currentTime - startTime;

        if (elapsedTime >= animationDuration) {
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < SCREEN_HEIGHT; y += 2) {
            for (int x = 0; x < SCREEN_WIDTH; x += 2) {
                float noiseValue = noise.GetNoise((x + camera.cameraPosition.x) * 30.0f, (y + camera.cameraPosition.y) * 30.0f);

                if (noiseValue > 0.75f) {
                    SDL_SetRenderDrawColor(renderer, 225, 225, 225, 225);

                    if (x < SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y - elapsedTime / 100, x, y );

                    else if (x > SCREEN_WIDTH / 2 && y < SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y - elapsedTime / 100, x, y);

                    else if (x < SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x - elapsedTime / 100, y + elapsedTime / 100, x, y);

                    else if (x > SCREEN_WIDTH / 2 && y > SCREEN_HEIGHT / 2)
                        SDL_RenderDrawLine(renderer, x + elapsedTime / 100, y + elapsedTime / 100, x, y);
                }
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(10);
    }
}

int main(int argv, char** args)
{
    if (!init()) {
        return 1;
    }

    clear(10, 10);

    float cameraSpeed = 0.3f;

    Camera camera;
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 15.0f);
    camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    uniforms.projection = createProjectionMatrix();

    uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);

    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Rotar alrededor del eje Y

    Planet sun;
    sun.planet = "sun";
    sun.worldPos = {0.0f, 0.0f, 0.0f};
    sun.rotAngle = 0.0f;
    sun.scaleFactor = {1.5f, 1.5f, 1.5f};
    sun.transRadius = 0.0f;
    sun.axis = {0.0f, 0.0f, 0.0f};

    Planet bluey;
    bluey.planet = "bluey";
    bluey.transRadius = 2.0f;
    bluey.rotAngle = 1.0f;
    bluey.scaleFactor = {1.0f, 1.0f, 1.0f};
    bluey.transAngle = 170.0f;
    bluey.speed = 1.0f;
    bluey.worldPos = {bluey.transRadius * cos(glm::radians(bluey.transAngle)), 0.0f, bluey.transRadius * sin(glm::radians(bluey.transAngle))};
    bluey.axis = sun.worldPos;

    Planet earth;
    earth.planet = "earth";
    earth.transRadius = 4.0f;
    earth.rotAngle = 1.0f;
    earth.scaleFactor = {1.0f, 1.0f, 1.0f};
    earth.transAngle = 100.0f;
    earth.worldPos = {earth.transRadius * cos(glm::radians(earth.transAngle)), 0.0f, earth.transRadius * sin(glm::radians(earth.transAngle))};
    earth.speed = 0.5f;
    earth.axis = sun.worldPos;

    Planet magneto;
    magneto.planet = "magneto";
    magneto.transRadius = 5.6f;
    magneto.rotAngle = 1.0f;
    magneto.scaleFactor = {2.0f, 2.0f, 2.0f};
    magneto.transAngle = 4.3f;
    magneto.speed = 1.7f;
    magneto.worldPos = {magneto.transRadius * cos(glm::radians(magneto.transAngle)), 0.0f, magneto.transRadius * sin(glm::radians(magneto.transAngle))};
    magneto.axis = sun.worldPos;

    Planet gas;
    gas.planet = "gas";
    gas.transRadius = 13.0f;
    gas.rotAngle = 4.0f;
    gas.scaleFactor = {1.8f, 1.8f, 1.8f};
    gas.transAngle = 325.6f;
    gas.speed = 0.5f;
    gas.worldPos = {gas.transRadius * cos(glm::radians(gas.transAngle)), 0.0f, gas.transRadius * sin(glm::radians(gas.transAngle))};
    gas.axis = sun.worldPos;

    Planet discoball;
    discoball.planet = "discoball";
    discoball.transRadius = 15.0f;
    discoball.rotAngle = 0.0f;
    discoball.scaleFactor = {1.3f, 1.6f, 1.1f};
    discoball.transAngle = 300.0f;
    discoball.speed = 0.3f;
    discoball.worldPos = {discoball.transRadius * cos(glm::radians(discoball.transAngle)), 0.0f, discoball.transRadius * sin(glm::radians(discoball.transAngle))};
    discoball.axis = sun.worldPos;


    Planet greeny;
    greeny.planet = "greeny";
    greeny.transRadius = 20.0f;
    greeny.rotAngle = 0.0f;
    greeny.scaleFactor = {1.8f, 1.8f, 1.8f};
    greeny.transAngle = 280.0f;
    greeny.speed = 0.9f;
    greeny.worldPos = {greeny.transRadius * cos(glm::radians(greeny.transAngle)), 0.0f, greeny.transRadius * sin(glm::radians(greeny.transAngle))};
    greeny.axis = sun.worldPos;

    Planet moon;
    moon.planet = "moon";
    moon.transRadius = 1.0f;
    moon.rotAngle = 0.0f;
    moon.scaleFactor = {0.2f, 0.2f, 0.2f};
    moon.transAngle = 1.0f;
    moon.speed = 2.0f;
    moon.axis = earth.worldPos;
    moon.worldPos = {moon.axis.x + (moon.transRadius * cos(glm::radians(moon.transAngle))),
                    0.3f,
                     moon.axis.z + (moon.transRadius * sin(glm::radians(moon.transAngle)))};

    earth.satellites.push_back(moon);

    sun.satellites.push_back(bluey);
    sun.satellites.push_back(earth);
    sun.satellites.push_back(earth);
    sun.satellites.push_back(magneto);
    sun.satellites.push_back(gas);
    sun.satellites.push_back(discoball);
    sun.satellites.push_back(greeny);

    Nave spaceship;
    spaceship.worldPos = {0.0f, 0.0f, 2.0f};
    spaceship.scaleFactor = {0.03f, 0.03f, 0.03f};
    spaceship.rotationAngle = 90.0f;
    spaceship.rotSpeed = 4.0f;
    spaceship.movSpeed = 1.8f;

    loadOBJ(navePath, naveVertices, naveNormals, naveFaces);
    naveVerticesArray = setupVertexArray(naveVertices, naveNormals, naveFaces);

    loadOBJ(planetPath, vertices, normals, faces);
    verticesArray = setupVertexArray(vertices, normals, faces);


    bool isRunning = true;
    while (isRunning) {
        frame += 1;
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                        if (spaceship.rotationAngle - spaceship.rotSpeed == 0.0f) {
                            spaceship.rotationAngle = 360.0f;
                        }
                        else {
                            spaceship.rotationAngle -= spaceship.rotSpeed;
                        }
                        break;

                    case SDLK_LEFT:
                        if (spaceship.rotationAngle + spaceship.rotSpeed > 360.0f) {
                            spaceship.rotationAngle = spaceship.rotSpeed;
                        }
                        else {
                            spaceship.rotationAngle += spaceship.rotSpeed;
                        }
                        break;

                    case SDLK_UP:
                        spaceship.worldPos.z -= spaceship.movSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x += spaceship.movSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_DOWN:
                        spaceship.worldPos.z += spaceship.movSpeed * sin(glm::radians(spaceship.rotationAngle));
                        spaceship.worldPos.x -= spaceship.movSpeed * cos(glm::radians(spaceship.rotationAngle));
                        break;

                    case SDLK_r:
                        fastTravel(renderer, camera);
                        spaceship.worldPos = {0.0f, 0.0f, 2.0f};
                        spaceship.rotationAngle = 90.0f;
                        break;

                    default:
                        break;
                }
            }
        }

        clear(10, 10);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);


            uniforms.view = glm::lookAt(
                camera.cameraPosition, // The position of the camera
                camera.targetPosition, // The point the camera is looking at
                camera.upVector        // The up vector defining the camera's orientation
            );

        setUpRender(sun);
        render(Primitive::TRIANGLES, sun.planet, verticesArray, sun.worldPos);

        glm::mat4 spaceshipTranslation = glm::translate(glm::mat4(1.0f), spaceship.worldPos);
        glm::mat4 spaceshipScale = glm::scale(glm::mat4(1.0f), spaceship.scaleFactor);
        glm::mat4 spaceshipRotation = glm::rotate(glm::mat4(1.0f), glm::radians(spaceship.rotationAngle), rotationAxis);
        uniforms.model = spaceshipTranslation * spaceshipRotation *  spaceshipScale;

        float d = 5.0f; //Distancia de camara a nave

        // Determinar posición de la cámara
        float cameraAngle;
        float dx;
        float dz;

        if (spaceship.rotationAngle > 0 && spaceship.rotationAngle <= 90) {
            cameraAngle = spaceship.rotationAngle;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 90 && spaceship.rotationAngle <= 180) {
            cameraAngle = spaceship.rotationAngle - 90.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z + dz};
        }

        else if (spaceship.rotationAngle > 180 && spaceship.rotationAngle <= 270) {
            cameraAngle = spaceship.rotationAngle - 180.0f;
            dz = d * sin(glm::radians(cameraAngle));
            dx = d * cos(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x + dx, 1.0f, spaceship.worldPos.z - dz};
        }

        else if (spaceship.rotationAngle > 270 && spaceship.rotationAngle <= 360) {
            cameraAngle = spaceship.rotationAngle - 270.0f;
            dz = d * cos(glm::radians(cameraAngle));
            dx = d * sin(glm::radians(cameraAngle));
            camera.cameraPosition = {spaceship.worldPos.x - dx, 1.0f, spaceship.worldPos.z - dz};
        }

        camera.targetPosition = spaceship.worldPos;

        render(Primitive::TRIANGLES, "ship", naveVerticesArray, spaceship.worldPos);


        for (Planet& planet : sun.satellites) {
            for (Planet& satellite : planet.satellites) {
                satellite.axis = planet.worldPos;
                setUpRender(satellite);
                render(Primitive::TRIANGLES, satellite.planet, verticesArray, satellite.worldPos);
                renderBuffer(renderer);
            }

            setUpRender(planet);
            render(Primitive::TRIANGLES, planet.planet, verticesArray, planet.worldPos);
            renderBuffer(renderer);

        }

        SDL_RenderPresent(renderer);

    }

    return 0;
}
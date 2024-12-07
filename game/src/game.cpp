#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>
#include <vector>

#include "shader.hpp"
#include "camera.hpp"

// #define DEBUG

/* -------------------------------------------------------------------------- */
/*                                  Settings                                  */
/* -------------------------------------------------------------------------- */

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const char* WINDOW_NAME = "Art Gallery";

/* ---------------------------------- Room ---------------------------------- */
const float roomSize = 10.0f;
const float roomHeightFactor = 0.45f;


/* --------------------------------- Player --------------------------------- */
Camera camera(glm::vec3(0.0f, 2.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

const float collisionPadding = 0.1;
glm::vec4 cameraCollisonBounds(glm::vec4(-1, 1, -1, 1)* (roomSize * 0.5f)* (1.0f - collisionPadding));


/* ---------------------------------- Time ---------------------------------- */
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


GLFWwindow* createWindow();
void setGlGlobalSettings();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadTexture(char const* path, int* width, int* height);
void processCameraCollision(Camera* camera);
void setLights(std::vector<glm::vec3>* lightPositions, Shader* shader);

enum SampleSpace {
    TEXCOORDS,
    XZ,
    XY,
    ZY,
};

struct Painting {
    unsigned int artDiffuseTexture{};
    unsigned int artSpecularTexture{};
    glm::vec3 size{};

    Painting(char const* diffusePath, char const* specularPath)
    {
        int width, height;

        artDiffuseTexture = loadTexture(diffusePath, &width, &height);
        artSpecularTexture = loadTexture(specularPath, &width, &height);
        size = glm::vec3((float)width / 64.0f, (float)height / 64.0f, 1.0f);

        std::cout << size.x << " " << size.y << '\n';
    }

    Painting(char const* path) : Painting(path, path) {}
}; // struct Painting


int main()
{
    /* ------------------- Create OpenGL Context and Windowing ------------------ */
    GLFWwindow* mainWindow = createWindow();
    setGlGlobalSettings();


    /* ---------------------------- Create Materials ---------------------------- */

    // Floor Material
    unsigned int floorDiffuseTexture = loadTexture("resources/textures/enviroment/floor.jpg");
    unsigned int floorSpecularTexture = loadTexture("resources/textures/enviroment/floor.jpg");
    Shader floorShader("src/shaders/default.vert", "src/shaders/default.frag");
    floorShader.use();
    floorShader.setInt("material.diffuse", 0);
    floorShader.setInt("material.specular", 1);
    floorShader.setFloat("material.shininess", 32.0f);
    floorShader.setInt("material.sampleSpace", 1);
    floorShader.setVec3("material.scale", glm::vec3(1.0f));
    floorShader.setVec3("material.translate", glm::vec3(0.0f));

    // Wall Material
    unsigned int wallDiffuseTexture = loadTexture("resources/textures/enviroment/wall.jpg");
    unsigned int wallSpecularTexture = loadTexture("resources/textures/enviroment/wall.jpg");
    Shader wallShader("src/shaders/default.vert", "src/shaders/default.frag");
    wallShader.use();
    wallShader.setInt("material.diffuse", 0);
    wallShader.setInt("material.specular", 1);
    wallShader.setFloat("material.shininess", 14.0f);
    wallShader.setInt("material.sampleSpace", 2);
    // wallShader.setVec3("material.scale", glm::vec3(0.5f, 0.75f, 0.5f));
    wallShader.setVec3("material.scale", glm::vec3(1.f));
    wallShader.setVec3("material.translate", glm::vec3(0.0f));

    // Ceiling Material
    unsigned int ceilingDiffuseTexture = loadTexture("resources/textures/enviroment/ceiling.jpg");
    unsigned int ceilingSpecularTexture = loadTexture("resources/textures/enviroment/ceiling.jpg");
    Shader ceilingShader("src/shaders/default.vert", "src/shaders/default.frag");
    ceilingShader.use();
    ceilingShader.setInt("material.diffuse", 0);
    ceilingShader.setInt("material.specular", 1);
    ceilingShader.setFloat("material.shininess", 16.0f);
    ceilingShader.setInt("material.sampleSpace", 3);
    ceilingShader.setVec3("material.scale", glm::vec3(1.0f));
    ceilingShader.setVec3("material.translate", glm::vec3(0.0f));

    // Painting
    std::vector<Painting> paintings;
    paintings.push_back(Painting("resources/textures/art/starry-night.jpg"));
    paintings.push_back(Painting("resources/textures/art/micheal.jpg"));
    paintings.push_back(Painting("resources/textures/art/mona-lisa.jpg"));
    //paintings.push_back(Painting("resources/textures/art/girl.jpg"));
    paintings.push_back(Painting("resources/textures/art/wave.jpg"));

    Shader paintingShader("src/shaders/default.vert", "src/shaders/default.frag");
    paintingShader.use();
    paintingShader.setInt("material.diffuse", 0);
    paintingShader.setInt("material.specular", 1);
    paintingShader.setFloat("material.shininess", 1.8f);
    paintingShader.setInt("material.sampleSpace", 2);
    paintingShader.setVec3("material.scale", glm::vec3(1.0f));
    paintingShader.setVec3("material.translate", glm::vec3(0.0f));

    /* --------------------------- Primitives Vertcies -------------------------- */
    // layout: Pos vec3, Normals vec3, TexCoords vec2 

    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    float planeUpVertices[] = {
         0.5f, 0.0f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, 0.0f,  0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, 1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 0.0f,
    };

    float planeDownVertices[] = {
         0.5f, 0.0f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, 0.0f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    };



    /* ----------------------------- Light Positions ---------------------------- */

    std::vector<glm::vec3> LightPositions = {
        glm::vec3(0.0f, roomSize * roomHeightFactor, 0.0f),
        glm::vec3(roomSize * 0.45, roomSize * roomHeightFactor, 0.0f),
        glm::vec3(-roomSize * 0.45, roomSize * roomHeightFactor,  0.0f),
        glm::vec3(0.0f, roomSize * roomHeightFactor,  roomSize * 0.45),
        glm::vec3(0.0f, roomSize * roomHeightFactor,  -roomSize * 0.45)
    };


    /* ----------------------- Create VAOs From Primitives ---------------------- */

    // Cube
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Plane
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeUpVertices), &planeUpVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);


    /* -------------------------------------------------------------------------- */
    /*                                  Main Loop                                 */
    /* -------------------------------------------------------------------------- */
    while (!glfwWindowShouldClose(mainWindow))
    {
        // Update time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // Window and Player Input
        processInput(mainWindow);

#ifndef DEBUG
        processCameraCollision(&camera);
#endif // !DEBUG



        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* -------------------------------------------------------------------------- */
        /*                                Render Scene                                */
        /* -------------------------------------------------------------------------- */

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model;
        glm::mat4 tranMat;
        glm::mat4 rotMat;
        glm::mat4 scaMat;


        /* ---------------------------------- Floor --------------------------------- */
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(roomSize));
        floorShader.use();
        floorShader.setMat4("model", model);
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        floorShader.setVec3("viewPos", camera.Position);
        setLights(&LightPositions, &floorShader);

        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorSpecularTexture);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /* --------------------------------- Ceiling -------------------------------- */
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, roomSize * roomHeightFactor, 0.0));
        model = glm::scale(model, glm::vec3(roomSize));
        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(1, 0, 0));
        ceilingShader.use();
        ceilingShader.setMat4("model", model);
        ceilingShader.setMat4("view", view);
        ceilingShader.setMat4("projection", projection);
        ceilingShader.setVec3("viewPos", camera.Position);
        setLights(&LightPositions, &ceilingShader);

        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceilingDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ceilingSpecularTexture);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);


        /* ---------------------------------- Walls ---------------------------------- */
        wallShader.use();
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallDiffuseTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallSpecularTexture);

        wallShader.setMat4("view", view);
        wallShader.setMat4("projection", projection);
        wallShader.setVec3("viewPos", camera.Position);
        setLights(&LightPositions, &wallShader);


        tranMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, roomSize * roomHeightFactor, roomSize) * 0.5f);
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        scaMat = glm::scale(glm::mat4(1.0f), glm::vec3(roomSize, roomSize, roomSize * roomHeightFactor));   
        model = tranMat * rotMat * scaMat;

        for (int i = 0; i < 4; i++)
        {
            wallShader.setMat4("model", glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * i), glm::vec3(0, 1, 0)) * model);
            wallShader.setInt("material.sampleSpace", i % 2 == 0 ? SampleSpace::XY : SampleSpace::ZY);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);

        /* ------------------------------ Art Paintings ----------------------------- */
        paintingShader.use();
        glBindVertexArray(planeVAO);

        paintingShader.setMat4("view", view);
        paintingShader.setMat4("projection", projection);
        paintingShader.setVec3("viewPos", camera.Position);
        setLights(&LightPositions, &paintingShader);

        tranMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, roomSize * roomHeightFactor, roomSize * 0.99) * 0.5f);
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));

        for (int i = 0; i < 4; i++)
        {
            Painting paintingCurr = paintings[i];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, paintingCurr.artDiffuseTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, paintingCurr.artSpecularTexture);

            scaMat = glm::scale(glm::mat4(1.f), paintingCurr.size * 2.0f);
            model = tranMat * scaMat * rotMat;
            model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * i), glm::vec3(0, 1, 0)) * model;

            paintingShader.setMat4("model", model);

            paintingShader.setInt("material.sampleSpace", SampleSpace::TEXCOORDS);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(mainWindow);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void setLights(std::vector<glm::vec3>* lightPositions, Shader* shader)
{
    float t = glfwGetTime();
    glm::vec3 w1(cos(t + 0.2), sin(t + 0.86), cos(t + 0.35));
    glm::vec3 w2(cos(t * 2 + 0.54), cos(t * 2 + 0.32), cos(t * 2 + 0.83));
    glm::vec3 w3(cos(t * 2 * 2 + 0.2), sin(t * 2 * 2 + 0.86), cos(t * 2 * 2 + 0.35));
    glm::vec3 noise = w1 * 0.33f + w2 * 0.33f + w3 * 0.33f;

    /* ------------------------------- Floor Light ------------------------------ */
    shader->setVec3("lights[0].position", glm::vec3(0.0f, 2.0f, 0.0f));
    shader->setVec3("lights[0].direction", glm::vec3(0.0f, -1.0f, 0.0f) + noise * 0.02f);
    shader->setFloat("lights[0].cutOff", glm::cos(glm::radians(20.f)));
    shader->setFloat("lights[0].outerCutOff", glm::cos(glm::radians(75.f - (sin(glfwGetTime() * 2) + 1) * 0.5)));
    shader->setVec3("lights[0].ambient", 0.1f, 0.1f, 0.2f);
    shader->setVec3("lights[0].diffuse", 0.60f * 1.0f, 0.50f * 1.0f, 0.30f * 1.0f);
    shader->setVec3("lights[0].specular", 1.0f * 2.0f, 1.0f * 2.0f, 1.0f * 2.0f);
    shader->setFloat("lights[0].constant", 1.0f);
    shader->setFloat("lights[0].linear", 0.08f);
    shader->setFloat("lights[0].quadratic", 0.016f);

    /* ----------------------------- Painting Lights ---------------------------- */
    for (int i = 1; i < lightPositions->size(); i++)
    {
        std::string idx = "lights[" + std::to_string(i) + "]";

        shader->setVec3(idx + ".position", lightPositions->at(i) + glm::vec3(0.0f, 0.1f, 0.0f));

        glm::vec3 direction = glm::normalize(glm::vec3(0.f, -5.f, 0.f) - glm::normalize(lightPositions->at(i)) * glm::vec3(-1, 0, -1));

        shader->setVec3(idx + ".direction", direction + noise * 0.01f);
        shader->setFloat(idx + ".cutOff", glm::cos(glm::radians(0.f)));
        shader->setFloat(idx + ".outerCutOff", glm::cos(glm::radians(35.f - (sin(glfwGetTime() * 2) + 1) * 0.5)));
        shader->setVec3(idx + ".ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3(idx + ".diffuse", 0.60f * 0.9f, 0.50f * 0.9f, 0.30f * 0.9f);
        shader->setVec3(idx + ".specular", 1.0f * 1.2f, 1.0f * 1.2f, 1.0f * 1.2f);
        shader->setFloat(idx + ".constant", 0.3f);
        shader->setFloat(idx + ".linear", 0.04f);
        shader->setFloat(idx + ".quadratic", 0.032f);
    }
}


void processCameraCollision(Camera* camera)
{
    // x-min
    if (camera->Position.x < cameraCollisonBounds.x)
        camera->Position.x = cameraCollisonBounds.x;
    // x-max
    if (camera->Position.x > cameraCollisonBounds.y)
        camera->Position.x = cameraCollisonBounds.y;
    // z-min
    if (camera->Position.z < cameraCollisonBounds.z)
        camera->Position.z = cameraCollisonBounds.z;
    // z-max
    if (camera->Position.z > cameraCollisonBounds.w)
        camera->Position.z = cameraCollisonBounds.w;
}


void processInput(GLFWwindow* window)
{
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* createWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = nullptr;
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_NAME, NULL, NULL);

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFWwindow" << std::endl;
        glfwTerminate();
        return window;
    }

    glfwMakeContextCurrent(window);

    // Input
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load Opengl Function Pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return window;
    }

    return window;
}

void setGlGlobalSettings()
{
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}


unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

    unsigned int loadTexture(char const* path, int* width, int* height)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int nrComponents;
        unsigned char* data = stbi_load(path, width, height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, *width, *height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }




#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
#include "dirLight.h"
#include "pointLight.h"
#include "spotLight.h"
#include "cube.h"
#include "cylinder.h"
#include "sphere.h"
#include "cone.h"
#include "bezier_mihrab.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// --- FUNCTION PROTOTYPES ---
glm::mat4 myPerspective(float fovRadians, float aspect, float near, float far);
glm::mat4 myOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
unsigned int loadTexture(char const *path, GLenum textureWrappingModeS, GLenum textureWrappingModeT, GLenum textureFilteringModeMin, GLenum textureFilteringModeMax);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Viewport enum
enum Viewport
{
    VP_ISOMETRIC = 0,
    VP_TOP = 1,
    VP_FRONT = 2,
    VP_INSIDE = 3
};
int activeViewport = VP_FRONT;
bool tabKeyPressed = false;

// Shading toggle (Phong fragment lighting <-> Gouraud vertex lighting)
bool gouraudShading = false;
bool gKeyPressed = false;

// 4 cameras   one per viewport
Camera cameras[4] = {
    // VP_ISOMETRIC: pulled back, above, looking at mosque
    Camera(glm::vec3(30.0f, 25.0f, 30.0f), glm::vec3(0.0f, 1.0f, 0.0f), -135.0f, -25.0f),
    // VP_TOP: directly above looking down
    Camera(glm::vec3(0.0f, 80.0f, -20.0f), glm::vec3(0.0f, 0.0f, -1.0f), -90.0f, -89.0f),
    // VP_FRONT: in front of mosque looking at it
    Camera(glm::vec3(0.0f, 10.0f, 48.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f),
    // VP_INSIDE: inside the mosque
    Camera(glm::vec3(0.0f, 8.0f, -80.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f)};

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float fov = 45.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- GLOBAL RENDER DATA ---
Cube *myCube = nullptr;
Cylinder *myCylinder = nullptr;
Sphere *mySphere = nullptr;
Cone *myCone = nullptr;
BezierMihrab *myBezierMihrab = nullptr;

const int NUM_POINT_LIGHTS = 36;

unsigned int texGrass = 0;
unsigned int texWall = 0;
unsigned int texPool = 0;
unsigned int texRoad = 0;
unsigned int texWood = 0;
unsigned int texFloor = 0;
unsigned int texWahdou = 0;
unsigned int texDome = 0;
unsigned int texDoorLeft = 0;
unsigned int texDoorRight = 0;
unsigned int texWindowLeft = 0;
unsigned int texWindowRight = 0;
unsigned int texMihrab = 0;
unsigned int texBook = 0;

// --- Global orbit variables ---
bool orbitMode = false;
bool fKeyPressed = false;
glm::vec3 orbitCenter;
float orbitRadius = 8.0f;
float orbitAngle = 0.0f;
float orbitSpeed = glm::radians(30.0f);

// --- Lighting toggle states ---
bool dirLightOn = true;
bool pointLightsOn = true;
bool spotLightsOn = true;
bool ambientOn = true;
bool diffuseOn = true;
bool specularOn = true;

// Key press tracking for toggles
bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool key4Pressed = false;
bool key5Pressed = false;
bool key6Pressed = false;
bool key7Pressed = false;
bool keyOPressed = false;
bool keyPPressed = false;
bool keyHPressed = false;

// Fan animation state
bool fansOn = false;
float fanAngleDeg = 0.0f;
float fanSpeedDegPerSec = 300.0f;

// Mosque roofline control (+10 units requested) and dependent vertical offsets.
constexpr float kMosqueRoofBaseY = 24.1f;
constexpr float kMosqueRoofRaiseY = 10.0f;
constexpr float kMosqueRoofY = kMosqueRoofBaseY + kMosqueRoofRaiseY;
constexpr float kGroundCenterX = 40.0f;
constexpr float kGroundCenterY = -0.15f;
constexpr float kGroundCenterZ = 20.0f;
constexpr float kGroundSizeX = 600.0f;
constexpr float kGroundSizeY = 0.3f;
constexpr float kGroundSizeZ = 600.0f;
constexpr float kGroundMinX = kGroundCenterX - (kGroundSizeX * 0.5f);
constexpr float kGroundMaxX = kGroundCenterX + (kGroundSizeX * 0.5f);
constexpr float kGroundMinZ = kGroundCenterZ - (kGroundSizeZ * 0.5f);
constexpr float kGroundMaxZ = kGroundCenterZ + (kGroundSizeZ * 0.5f);

constexpr float kRoadWidth = 18.0f;
constexpr float kRoadThickness = 0.03f;
constexpr float kRoadSurfaceY = 0.015f;
constexpr float kRoadLampFirstT = 0.30f;
constexpr float kRoadLampLastT = 0.70f;
constexpr float kRoadLampEdgeOffset = kRoadWidth * 0.5f;
constexpr float kFrontRoadCenterX = 40.0f;
constexpr float kFrontRoadStartZ = -5.0f;
constexpr float kFrontRoadEndZ = kGroundMaxZ;
constexpr float kRightRoadStartX = 160.0f;
constexpr float kRightRoadEndX = kGroundMaxX;
constexpr float kRightRoadCenterZ = -110.0f;

constexpr float kMosqueWallBaseY = 2.0f;
constexpr float kWallTopClearanceBelowRoof = 0.1f;
constexpr float kMosqueWallHeight = kMosqueRoofY - (kMosqueWallBaseY + kWallTopClearanceBelowRoof);
constexpr float kMosqueWallCenterY = kMosqueWallBaseY + (kMosqueWallHeight * 0.5f);

constexpr float kInteriorCeilingYOffsetFromRoof = -2.1f;
constexpr float kInteriorCeilingY = kMosqueRoofY + kInteriorCeilingYOffsetFromRoof;

constexpr float kVerandaLampYOffsetFromRoof = -0.25f;
constexpr float kVerandaLampY = kMosqueRoofY + kVerandaLampYOffsetFromRoof;

constexpr float kChandelierLampYOffsetFromRoof = -5.0f;
constexpr float kChandelierLampY = kMosqueRoofY + kChandelierLampYOffsetFromRoof;
constexpr float kSpotLightYOffsetFromRoof = -3.1f;
constexpr float kSpotLightY = kMosqueRoofY + kSpotLightYOffsetFromRoof;
constexpr float kInsidePendantYOffsetFromRoof = -9.1f;
constexpr float kInsidePendantY = kMosqueRoofY + kInsidePendantYOffsetFromRoof;

constexpr float kMainDomeYOffsetFromRoof = -0.22f;
constexpr float kMainDomeY = kMosqueRoofY + kMainDomeYOffsetFromRoof;
constexpr float kMainDomeOrnamentYOffsetFromRoof = 24.55f;
constexpr float kMainDomeOrnamentY = kMosqueRoofY + kMainDomeOrnamentYOffsetFromRoof;

constexpr float kMihrabDepth = 33.0f;
constexpr float kMihrabCenterZ = -58.5f;

constexpr float kSideWallOpeningHeight = 16.0f;
constexpr float kSideWindowSillHeight = 6.0f;
constexpr float kSideWindowLeafHeight = kSideWallOpeningHeight - kSideWindowSillHeight;
constexpr float kSideWindowY = kMosqueWallBaseY + kSideWindowSillHeight + (kSideWindowLeafHeight * 0.5f);

constexpr float kChandelierMainRodTopYOffsetFromRoof = -0.2f;
constexpr float kChandelierMainRodTopY = kMosqueRoofY + kChandelierMainRodTopYOffsetFromRoof;

constexpr float kFanRodTopYOffsetFromRoof = -2.2f;
constexpr float kFanRodBottomYOffsetFromRoof = -6.4f;
constexpr float kFanHubYOffsetFromRoof = -6.7f;
constexpr float kFanBladeYOffsetFromRoof = -6.75f;

// Door/window animation state
bool doorsOpen = false;
bool windowsOpen = false;
float doorOpenAmount = 0.0f;   // 0 closed, 1 open
float windowOpenAmount = 0.0f; // 0 closed, 1 open

// Corner bookshelf animation state
bool bookshelvesOpen = false;
float bookshelfOpenAmount = 0.0f;
bool keyBPressed = false;

// Faucet water particles
bool faucetWaterOn = false;
struct FaucetDroplet
{
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    int sourceIndex;
};
std::vector<glm::vec3> faucetSourceWorldPositions;
std::vector<FaucetDroplet> faucetDroplets;
bool faucetDropletsReady = false;

static void syncFaucetSources(const glm::mat4 &wadhuTransform);
static void updateFaucetDroplets(float dt);
static void renderFaucetDroplets(Shader &lightingShader, Sphere sphere);

// Calculates and returns a configurable point in 3D space indicating exactly where
// the active camera is currently looking, projected out by a specified distance.
glm::vec3 getLookAtPoint(float distance = 8.0f)
{
    return cameras[activeViewport].Position + cameras[activeViewport].Front * distance;
}

// Prints the interactive keyboard and mouse controls for the Mosque Scene
// to the standard output console for the user to read upon startup.
void printControls()
{
    std::cout << "========================================" << std::endl;
    std::cout << "       MOSQUE SCENE - CONTROLS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Viewport ---" << std::endl;
    std::cout << "  TAB        - Cycle active viewport" << std::endl;
    std::cout << "               (Isometric -> Top -> Front -> Inside)" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Movement ---" << std::endl;
    std::cout << "  W/S        - Move forward/backward" << std::endl;
    std::cout << "  A/D        - Move left/right" << std::endl;
    std::cout << "  E/R        - Move up/down" << std::endl;
    std::cout << "  Mouse      - Look around" << std::endl;
    std::cout << "  Scroll     - Zoom in/out" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Rotation ---" << std::endl;
    std::cout << "  X          - Rotate pitch (+Shift reverse)" << std::endl;
    std::cout << "  Y          - Rotate yaw   (+Shift reverse)" << std::endl;
    std::cout << "  Z          - Rotate roll  (+Shift reverse)" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Orbit ---" << std::endl;
    std::cout << "  F          - Toggle orbit mode" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Shading ---" << std::endl;
    std::cout << "  G          - Toggle shading (Phong/Gouraud)" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Light Type Toggles ---" << std::endl;
    std::cout << "  1          - Toggle Directional Light (sun)" << std::endl;
    std::cout << "  2          - Toggle Point Lights (lamp posts)" << std::endl;
    std::cout << "  3          - Toggle Spot Light (mosque hanging bulb)" << std::endl;
    std::cout << "  4          - Toggle Mosque Ceiling Fans" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Light Component Toggles ---" << std::endl;
    std::cout << "  5          - Toggle Ambient component" << std::endl;
    std::cout << "  6          - Toggle Diffuse component" << std::endl;
    std::cout << "  7          - Toggle Specular component" << std::endl;
    std::cout << std::endl;
    std::cout << "--- Openable Elements ---" << std::endl;
    std::cout << "  O          - Open/Close mosque door" << std::endl;
    std::cout << "  P          - Open/Close mosque window" << std::endl;
    std::cout << "  H          - Toggle faucet water flow" << std::endl;
    std::cout << "  B          - Open/Close corner bookshelves" << std::endl;
    std::cout << std::endl;
    std::cout << "  ESC        - Quit" << std::endl;
    std::cout << "========================================" << std::endl;
}

// Loads a texture image file from the specified path using the stb_image library,
// generates an OpenGL texture object, configures wrapping and filtering parameters,
// and returns the texture ID.
unsigned int loadTexture(char const *path, GLenum textureWrappingModeS, GLenum textureWrappingModeT, GLenum textureFilteringModeMin, GLenum textureFilteringModeMax, bool flipVertically = true)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(flipVertically);
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrappingModeS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrappingModeT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilteringModeMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilteringModeMax);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Forward declarations for scene drawing
void drawScene(Shader &lightingShader, Shader &lightCubeShader,
               glm::mat4 projection, glm::mat4 view, glm::vec3 camPos,
               glm::vec3 wallColor, glm::vec3 domeColor, glm::vec3 minaretColor,
               glm::vec3 roofColor, glm::vec3 doorColor, glm::vec3 rimColor,
               glm::vec3 pillarColor,
               glm::vec3 waterColor, glm::vec3 goldColor, glm::vec3 tileColor,
               PointLight *pointLights[], DirLight *dirLight, SpotLight *spotLight,
               glm::vec3 lampLightColor, glm::vec3 spotLightPos, glm::vec3 spotLightColor,
               glm::vec3 pondPos, float pondW, float pondD, float pondWallH, float pondWallT,
               glm::vec3 pondCornerPositions[], glm::vec3 pondCornerColors[],
               int numLampPairs, float lampFirst, float lampStep, float lampXOffset,
               int numRightLampPairs, float rightLampFirst, float rightLampStep, float rightLampZOffset);

// The main entry point of the application. It initializes GLFW, creates the window,
// loads OpenGL extensions via GLAD, compiles shaders, loads textures, sets up lights,
// and runs the main rendering loop for the 3D scene.
int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simple Mosque", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Print controls to console
    printControls();

    // build and compile our shader programs
    Shader lightingShader("lighting.vs", "lighting.fs");
    Shader gouraudLightingShader("gouraud.vs", "gouraud.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");

    texGrass = loadTexture("grass.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texWall = loadTexture("wall.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texPool = loadTexture("pool_water.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texRoad = loadTexture("road.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texWood = loadTexture("wood.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texFloor = loadTexture("floor.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, false);
    texWahdou = loadTexture("wahdou.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texDome = loadTexture("dome1.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texDoorLeft = loadTexture("door_left.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texDoorRight = loadTexture("door_right.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texWindowLeft = loadTexture("window_left.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texWindowRight = loadTexture("window_right.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texMihrab = loadTexture("mihrab.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
    texBook = loadTexture("book_texture.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);

    lightingShader.use();
    lightingShader.setInt("diffuseMap", 0);
    lightingShader.setFloat("materialTexBlend", 0.0f);
    gouraudLightingShader.use();
    gouraudLightingShader.setInt("diffuseMap", 0);
    gouraudLightingShader.setFloat("materialTexBlend", 0.0f);

    // --- INITIALIZE SHAPES ONCE ---
    myCube = new Cube();
    myCylinder = new Cylinder(36);
    mySphere = new Sphere(18, 36);
    myCone = new Cone(36);
    myBezierMihrab = new BezierMihrab(-15.0f, 15.0f, -48.5f, kMihrabDepth, kMosqueWallBaseY, 3.0f, kMosqueRoofY, 48);

    orbitCenter = glm::vec3(0.0f, 0.0f, 6.0f);

    // --- Color palette ---
    glm::vec3 groundColor(0.82f, 0.92f, 0.80f);
    glm::vec3 wallColor(0.96f, 0.93f, 0.86f);
    glm::vec3 domeColor(0.92f, 0.96f, 1.00f);

    glm::vec3 minaretColor(0.93f, 0.91f, 0.84f);
    glm::vec3 roofColor(0.84f, 0.88f, 0.84f);
    glm::vec3 doorColor(0.66f, 0.49f, 0.31f);
    glm::vec3 rimColor(0.84f, 0.89f, 0.95f);
    glm::vec3 waterColor(0.58f, 0.80f, 0.94f);
    glm::vec3 waterDeepColor(0.23f, 0.46f, 0.72f);
    glm::vec3 pillarColor(0.95f, 0.92f, 0.86f);
    glm::vec3 goldColor(0.92f, 0.77f, 0.32f);
    glm::vec3 tileColor(0.90f, 0.92f, 0.89f);
    glm::vec3 darkGreen(0.16f, 0.36f, 0.22f);

    // --- Object Abstraction setup ---
    DirLight *mainDirLight = new DirLight(
        glm::vec3(-0.2f, -1.0f, -0.3f), // direction
        glm::vec3(0.15f, 0.15f, 0.15f), // ambient
        glm::vec3(0.6f, 0.6f, 0.55f),   // diffuse
        glm::vec3(0.4f, 0.4f, 0.4f)     // specular
    );

    glm::vec3 lampLightColor(1.0f, 0.9f, 0.5f);

    // --- Point light positions (6 road lamps + 2 inside + all veranda lamps + 1 wadhu) ---
    float mosqueZ_init = -80.0f;
    float pathStartZ_init = kFrontRoadStartZ;
    float pathEndZ_init = kFrontRoadEndZ;
    float pathLen_init = pathEndZ_init - pathStartZ_init;
    float lampXOffset = kRoadLampEdgeOffset;
    float lampFirst = pathStartZ_init + pathLen_init * kRoadLampFirstT;
    float lampLast = pathStartZ_init + pathLen_init * kRoadLampLastT;
    int numLampPairs = 3;
    float lampStep = (lampLast - lampFirst) / (numLampPairs - 1);

    PointLight *myPointLights[NUM_POINT_LIGHTS];
    for (int i = 0; i < numLampPairs; ++i)
    {
        float lz = lampFirst + i * lampStep;
        myPointLights[i * 2 + 0] = new PointLight(
            glm::vec3(40.0f - lampXOffset, 10.0f, lz),
            glm::vec3(0.1f, 0.1f, 0.08f), // increased ambient
            lampLightColor * 1.0f,
            glm::vec3(1.0f, 0.95f, 0.8f), // specular
            1.0f, 0.022f, 0.0019f,        // decreased attenuation to cover more area
            i * 2 + 0);
        myPointLights[i * 2 + 1] = new PointLight(
            glm::vec3(40.0f + lampXOffset, 10.0f, lz),
            glm::vec3(0.1f, 0.1f, 0.08f), // increased ambient
            lampLightColor * 1.0f,
            glm::vec3(1.0f, 0.95f, 0.8f), // specular
            1.0f, 0.022f, 0.0019f,        // decreased attenuation to cover more area
            i * 2 + 1);
    }
    // Two inside-mosque point lights (kept similar to the earlier look).
    // These are separate from the chandelier ring lamps (added later).
    myPointLights[6] = new PointLight(
        glm::vec3(5.0f, kInsidePendantY, -110.0f),
        glm::vec3(0.11f, 0.10f, 0.07f),
        lampLightColor * 1.0f,
        glm::vec3(0.85f, 0.78f, 0.62f),
        1.0f, 0.022f, 0.0013f, 6);
    myPointLights[7] = new PointLight(
        glm::vec3(75.0f, kInsidePendantY, -110.0f),
        glm::vec3(0.11f, 0.10f, 0.07f),
        lampLightColor * 1.0f,
        glm::vec3(0.85f, 0.78f, 0.62f),
        1.0f, 0.022f, 0.0013f, 7);

    // Veranda point lights (actual illumination for veranda hemispherical lamps)
    int nextPointLight = 8;

    // Front veranda lamps: all centers between front pillar groups
    std::vector<float> frontPillarXs = {-30.0f, 5.0f, 40.0f, 75.0f, 110.0f, 145.0f};
    std::vector<float> frontPillarZs = {mosqueZ_init + 25.0f, mosqueZ_init + 45.0f, mosqueZ_init + 65.0f};
    for (size_t ix = 0; ix + 1 < frontPillarXs.size(); ++ix)
    {
        for (size_t iz = 0; iz + 1 < frontPillarZs.size(); ++iz)
        {
            float px = (frontPillarXs[ix] + frontPillarXs[ix + 1]) * 0.5f;
            float pz = (frontPillarZs[iz] + frontPillarZs[iz + 1]) * 0.5f;
            myPointLights[nextPointLight] = new PointLight(
                glm::vec3(px, kVerandaLampY, pz),
                glm::vec3(0.06f, 0.055f, 0.045f),
                glm::vec3(1.0f, 0.92f, 0.75f) * 0.85f,
                glm::vec3(1.0f, 0.95f, 0.85f),
                1.0f, 0.028f, 0.0032f,
                nextPointLight);
            ++nextPointLight;
        }
    }

    // Right veranda lamps: all centers between right veranda pillar spans
    std::vector<float> rightPillarZs = {mosqueZ_init - 75.0f, mosqueZ_init - 45.0f, mosqueZ_init - 15.0f, mosqueZ_init + 19.0f};
    for (size_t i = 0; i + 1 < rightPillarZs.size(); ++i)
    {
        float pz = (rightPillarZs[i] + rightPillarZs[i + 1]) * 0.5f;
        myPointLights[nextPointLight] = new PointLight(
            glm::vec3(135.0f, kVerandaLampY, pz),
            glm::vec3(0.06f, 0.055f, 0.045f),
            glm::vec3(1.0f, 0.92f, 0.75f) * 0.9f,
            glm::vec3(1.0f, 0.95f, 0.85f),
            1.0f, 0.028f, 0.0032f,
            nextPointLight);
        ++nextPointLight;
    }

    // Single wadhu point light
    myPointLights[nextPointLight] = new PointLight(
        glm::vec3(-59.0f, kMosqueRoofY, -35.5f),
        glm::vec3(0.08f, 0.08f, 0.10f),
        glm::vec3(0.75f, 0.80f, 1.0f) * 1.25f,
        glm::vec3(0.9f, 0.95f, 1.0f),
        1.0f, 0.030f, 0.0035f,
        nextPointLight);
    ++nextPointLight;

    // --- Chandelier ring point lights (at least 4, arranged circularly) ---
    // These are new point lights around the central hanging spot-light fixture.
    const int chandelierCount = 4;
    const float chandelierRingRadius = 3.0f;
    const float chandelierLampY = kChandelierLampY;
    const float chandelierZ = mosqueZ_init - 30.0f;
    const glm::vec3 chandelierCenterWorld = glm::vec3(40.0f, chandelierLampY, chandelierZ);

    for (int i = 0; i < chandelierCount; ++i)
    {
        float a = (2.0f * (float)M_PI) * ((float)i / (float)chandelierCount);
        glm::vec3 p = chandelierCenterWorld + glm::vec3(std::cos(a) * chandelierRingRadius, 0.0f, std::sin(a) * chandelierRingRadius);

        myPointLights[nextPointLight] = new PointLight(
            p,
            glm::vec3(0.08f, 0.075f, 0.055f),
            lampLightColor * 1.0f,
            glm::vec3(0.85f, 0.78f, 0.62f),
            1.0f, 0.020f, 0.0011f,
            nextPointLight);
        ++nextPointLight;
    }

    // --- Right road lamp pairs ---
    // Right road extends from the right veranda stairs outward in +X direction.
    // The right veranda edge is at mosqueRoot X + 110 = 40 + 110 = 150 world X.
    // Stairs go from 150 to ~158. Road starts after stairs.
    float rightRoadStartX = kRightRoadStartX;
    float rightRoadEndX = kRightRoadEndX;
    float rightRoadLen = rightRoadEndX - rightRoadStartX;
    float rightLampZOffset = kRoadLampEdgeOffset;
    float rightLampFirstX = rightRoadStartX + rightRoadLen * kRoadLampFirstT;
    float rightLampLastX = rightRoadStartX + rightRoadLen * kRoadLampLastT;
    int numRightLampPairs = 3;
    float rightLampStepX = (rightLampLastX - rightLampFirstX) / (numRightLampPairs - 1);

    for (int i = 0; i < numRightLampPairs; ++i)
    {
        float lx = rightLampFirstX + i * rightLampStepX;
        // Road center Z is at mosqueRoot Z center = mosqueZ_init - 30 + 0 = -110 world...
        // Actually the right veranda stairs are at mosqueRoot local (110, y, 0), mosqueRoot = translate(40, 0, mosqueZ_init - 30)
        // So world position: stairs center Z = mosqueZ_init - 30 + 0 = -80 - 30 = -110
        // The road should extend at the same Z as the stair center (Z=0 in mosqueRoot local = -110 world)
        float roadCenterZ = kRightRoadCenterZ;
        myPointLights[nextPointLight] = new PointLight(
            glm::vec3(lx, 10.0f, roadCenterZ - rightLampZOffset),
            glm::vec3(0.1f, 0.1f, 0.08f),
            lampLightColor * 1.0f,
            glm::vec3(1.0f, 0.95f, 0.8f),
            1.0f, 0.022f, 0.0019f,
            nextPointLight);
        ++nextPointLight;
        myPointLights[nextPointLight] = new PointLight(
            glm::vec3(lx, 10.0f, roadCenterZ + rightLampZOffset),
            glm::vec3(0.1f, 0.1f, 0.08f),
            lampLightColor * 1.0f,
            glm::vec3(1.0f, 0.95f, 0.8f),
            1.0f, 0.022f, 0.0019f,
            nextPointLight);
        ++nextPointLight;
    }

    // Fill any remaining point lights with "off" lights to avoid uninitialized pointers.
    for (int i = nextPointLight; i < NUM_POINT_LIGHTS; ++i)
    {
        myPointLights[i] = new PointLight(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f),
            glm::vec3(0.0f),
            glm::vec3(0.0f),
            1.0f, 0.0f, 0.0f,
            i);
    }

    // --- Spot light: single hanging bulb inside mosque ---
    float mosqueZ_spot = mosqueZ_init - 30.0f; // Adjusted for deeper mosque center
    glm::vec3 spotLightPos = glm::vec3(40.0f, kSpotLightY, mosqueZ_spot);
    glm::vec3 spotLightColor = glm::vec3(0.9f, 0.9f, 0.85f);
    SpotLight *mainSpotLight = new SpotLight(
        spotLightPos,
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::cos(glm::radians(35.0f)),
        // Boost spotlight so it's clearly distinct from the chandelier point lamps.
        spotLightColor * 0.12f,
        spotLightColor * 2.0f,
        spotLightColor * 1.25f,
        1.0f, 0.027f, 0.0028f,
        0);

    // --- Pond dimensions ---
    glm::vec3 pondPos = glm::vec3(0.0f, 0.0f, 6.0f);
    float pondW = 5.0f;
    float pondD = 14.0f;
    float pondWallH = 0.12f;
    float pondWallT = 0.15f;

    // --- Emissive pond corner cube colors ---
    glm::vec3 pondCornerColors[4] = {
        glm::vec3(1.0f, 0.2f, 0.2f), // red
        glm::vec3(0.2f, 1.0f, 0.2f), // green
        glm::vec3(0.2f, 0.4f, 1.0f), // blue
        glm::vec3(1.0f, 1.0f, 0.2f)  // yellow
    };
    float cornerX[2] = {pondPos.x - pondW * 0.5f, pondPos.x + pondW * 0.5f};
    float cornerZ[2] = {pondPos.z - pondD * 0.5f, pondPos.z + pondD * 0.5f};
    glm::vec3 pondCornerPositions[4] = {
        glm::vec3(cornerX[0], pondWallH * 0.5f + 0.02f, cornerZ[0]),
        glm::vec3(cornerX[0], pondWallH * 0.5f + 0.02f, cornerZ[1]),
        glm::vec3(cornerX[1], pondWallH * 0.5f + 0.02f, cornerZ[0]),
        glm::vec3(cornerX[1], pondWallH * 0.5f + 0.02f, cornerZ[1])};


	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);
        updateFaucetDroplets(deltaTime);

        if (fansOn)
        {
            fanAngleDeg += fanSpeedDegPerSec * deltaTime;
            if (fanAngleDeg > 360.0f)
                fanAngleDeg -= 360.0f;
        }

        float openAnimSpeed = 1.8f;
        float doorTarget = doorsOpen ? 1.0f : 0.0f;
        float windowTarget = windowsOpen ? 1.0f : 0.0f;
        doorOpenAmount += (doorTarget - doorOpenAmount) * std::min(1.0f, openAnimSpeed * deltaTime * 3.0f);
        windowOpenAmount += (windowTarget - windowOpenAmount) * std::min(1.0f, openAnimSpeed * deltaTime * 3.0f);
        float bookshelfTarget = bookshelvesOpen ? 1.0f : 0.0f;
        bookshelfOpenAmount += (bookshelfTarget - bookshelfOpenAmount) * std::min(1.0f, openAnimSpeed * deltaTime * 3.0f);

        // render
        glClearColor(0.55f, 0.75f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texGrass);

        // Get current window size
        int winW, winH;
        glfwGetFramebufferSize(window, &winW, &winH);
        int halfW = winW / 2;
        int halfH = winH / 2;

        // Viewport aspect ratio
        float vpAspect = (float)winW / (float)winH;

        // Define view/projection for each viewport
        // Layout:  [0] Isometric (top-left)  | [1] Top (top-right)
        //          [2] Front (bottom-left)    | [3] Inside (bottom-right)
        struct ViewportInfo
        {
            int x, y, w, h;
            glm::mat4 projection;
            glm::mat4 view;
            glm::vec3 camPos;
        };

        ViewportInfo viewports[4];

        // Positions: top-left, top-right, bottom-left, bottom-right
        // viewports[VP_ISOMETRIC] = { 0,     halfH, halfW, halfH, glm::mat4(1.0f), glm::mat4(1.0f), glm::vec3(0.0f) };
        // viewports[VP_TOP]       = { halfW, halfH, halfW, halfH, glm::mat4(1.0f), glm::mat4(1.0f), glm::vec3(0.0f) };
        viewports[VP_FRONT] = {0, 0, winW, winH, glm::mat4(1.0f), glm::mat4(1.0f), glm::vec3(0.0f)};
        // viewports[VP_INSIDE]    = { halfW, 0,     halfW, halfH, glm::mat4(1.0f), glm::mat4(1.0f), glm::vec3(0.0f) };

        // Set up view and projection for each viewport
        for (int v = 2; v < 3; ++v)
        {
            Camera &cam = cameras[v];
            viewports[v].camPos = cam.Position;
            viewports[v].view = cam.GetViewMatrix();

            if (v == VP_TOP)
            {
                // Orthographic for top-down view
                float orthoSize = 45.0f;
                viewports[v].projection = myOrtho(-orthoSize * vpAspect, orthoSize * vpAspect,
                                                  -orthoSize, orthoSize, 0.1f, 200.0f);
            }
            else
            {
                viewports[v].projection = myPerspective(glm::radians(cam.Zoom), vpAspect, 0.1f, 1200.0f);
            }
        }

        // Draw the scene 4 times, once per viewport
        Shader &activeLightingShader = gouraudShading ? gouraudLightingShader : lightingShader;
        for (int v = 2; v < 3; ++v)
        {
            glViewport(viewports[v].x, viewports[v].y, viewports[v].w, viewports[v].h);
            glEnable(GL_SCISSOR_TEST);
            glScissor(viewports[v].x, viewports[v].y, viewports[v].w, viewports[v].h);

            drawScene(activeLightingShader, lightCubeShader,
                      viewports[v].projection, viewports[v].view, viewports[v].camPos,
                      wallColor, domeColor, minaretColor, roofColor, doorColor, rimColor,
                      pillarColor,
                      waterColor, goldColor, tileColor,
                      myPointLights, mainDirLight, mainSpotLight,
                      lampLightColor,
                      spotLightPos, spotLightColor,
                      pondPos, pondW, pondD, pondWallH, pondWallT,
                      pondCornerPositions, pondCornerColors,
                      numLampPairs, lampFirst, lampStep, lampXOffset,
                      numRightLampPairs, rightLampFirstX, rightLampStepX, rightLampZOffset);
        }

        glDisable(GL_SCISSOR_TEST);

        // Draw viewport border lines and labels using simple line rendering
        // Draw thin separator lines by rendering narrow quads (or just rely on the clear color gaps)
        // For a cleaner look, draw active viewport highlight border
        // (OpenGL line drawing - simple approach)
        glViewport(0, 0, winW, winH);
        glDisable(GL_DEPTH_TEST);
        // We'll skip fancy border drawing to keep it simple - the viewport edges are visible from the scene

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate
    delete myCube;
    delete myCylinder;
    delete mySphere;
    delete myCone;
    delete myBezierMihrab;
    delete mainDirLight;
    delete mainSpotLight;
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        delete myPointLights[i];
    }
    unsigned int allTextures[] = {
        texGrass, texWall, texPool, texRoad, texWood, texFloor, texWahdou, texDome,
        texDoorLeft, texDoorRight, texWindowLeft, texWindowRight, texMihrab, texBook};
    glDeleteTextures(14, allTextures);

    glfwTerminate();
    return 0;
}

// Helper function to create a new model transformation matrix by applying
// a local translation and scaling to a parent matrix.
static glm::mat4 composeModel(const glm::mat4 &parent, const glm::vec3 &localPos, const glm::vec3 &localScale)
{
    glm::mat4 m = parent;
    m = glm::translate(m, localPos);
    m = glm::scale(m, localScale);
    return m;
}

// This function calculates and stores the world positions of all the
// water faucets along the walls of the Wadhu (ablution) area.
static void syncFaucetSources(const glm::mat4 &wadhuTransform)
{
    faucetSourceWorldPositions.clear();

    const float pipeY = 5.45f;
    // LEFT wall faucets
    for (int i = 0; i < 4; ++i)
    {
        float zSeat = -10.0f - (i * 10.0f);
        glm::vec3 localTip(-18.38f, pipeY - 0.125f, zSeat);
        faucetSourceWorldPositions.push_back(glm::vec3(wadhuTransform * glm::vec4(localTip, 1.0f)));
    }
    // FRONT wall faucets
    for (int i = 0; i < 3; ++i)
    {
        float xSeat = -6.0f + (i * 10.0f);
        glm::vec3 localTip(xSeat, pipeY - 0.125f, -1.40f);
        faucetSourceWorldPositions.push_back(glm::vec3(wadhuTransform * glm::vec4(localTip, 1.0f)));
    }
    // BACK wall faucets
    for (int i = 0; i < 3; ++i)
    {
        float xSeat = -6.0f + (i * 10.0f);
        glm::vec3 localTip(xSeat, pipeY - 0.125f, -49.60f);
        faucetSourceWorldPositions.push_back(glm::vec3(wadhuTransform * glm::vec4(localTip, 1.0f)));
    }
}

// This function updates the physics (velocity and lifetime) of the water droplets
// falling from the faucets over time to simulate a continuous water flow.
static void updateFaucetDroplets(float dt)
{
    if (faucetSourceWorldPositions.empty())
    {
        return;
    }

    if (!faucetDropletsReady)
    {
        const int dropletsPerFaucet = 16;
        const int sourceCount = static_cast<int>(faucetSourceWorldPositions.size());
        faucetDroplets.resize(sourceCount * dropletsPerFaucet);
        for (size_t i = 0; i < faucetDroplets.size(); ++i)
        {
            FaucetDroplet &d = faucetDroplets[i];
            d.sourceIndex = static_cast<int>(i % sourceCount);
            d.position = faucetSourceWorldPositions[d.sourceIndex];
            d.velocity = glm::vec3(0.0f);
            d.lifetime = 0.0f;
        }
        faucetDropletsReady = true;
    }

    const float gravityAccel = 11.5f;
    const float channelY = 3.02f;
    for (FaucetDroplet &d : faucetDroplets)
    {
        if (d.lifetime <= 0.0f)
        {
            if (!faucetWaterOn)
            {
                continue;
            }
            glm::vec3 src = faucetSourceWorldPositions[d.sourceIndex];
            float jx = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.10f;
            float jz = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.10f;
            d.position = src + glm::vec3(jx, 0.0f, jz);
            d.velocity = glm::vec3(jx * 2.2f, -2.2f - (static_cast<float>(rand() % 100) / 100.0f), jz * 2.2f);
            d.lifetime = 0.45f + (static_cast<float>(rand() % 100) / 100.0f) * 0.55f;
            continue;
        }

        d.velocity.y -= gravityAccel * dt;
        d.position += d.velocity * dt;
        d.lifetime -= dt;
        if (d.position.y <= channelY)
        {
            d.lifetime = 0.0f;
        }
    }
}

// This function renders the active water droplet particles currently flowing
// out from the Wadhu faucets using scaled spheres to represent water droplets.
static void renderFaucetDroplets(Shader &lightingShader, Sphere *sphere)
{
    if (!faucetDropletsReady)
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, texPool);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    for (const FaucetDroplet &d : faucetDroplets)
    {
        if (d.lifetime <= 0.0f)
        {
            continue;
        }
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, d.position);
        m = glm::scale(m, glm::vec3(0.045f, 0.085f, 0.045f));
        sphere->draw(lightingShader, m, glm::vec3(0.8f, 2.0f, 0.8f)); // Draw one faucet water droplet particle.
    }
}

// This function constructs and draws the complete Wadhu (ablution) building and area.
// It draws the base structure, walls, seating blocks, water channels, fancy metal
// faucets, and the overhead hanging lamp.
static void drawWadhu(
    Shader &lightingShader,
    Shader &lightCubeShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    const glm::mat4 &wadhuTransform,
    const glm::mat4 &projection,
    const glm::mat4 &view,
    const glm::vec3 &wallColor,
    const glm::vec3 &tileColor)
{
    // Wadhu base
    glBindTexture(GL_TEXTURE_2D, texWahdou);
    lightingShader.setVec2("uvTiling", 6.0f, 6.0f);
    lightingShader.setFloat("materialTexBlend", 0.5f);
    glm::mat4 m = composeModel(wadhuTransform, glm::vec3(0.0f, 1.5f, -25.5f), glm::vec3(36.0f, 3.0f, 48.0f));
    myCube->draw(lightingShader, m, glm::vec3(0.58f, 0.76f, 1.18f)); // Draw the wadhu base floor block.
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // Wadhu water flow channels along three walls
    glBindTexture(GL_TEXTURE_2D, texPool);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

    const float channelTopY = 3.04f;
    const float channelBottomY = 0.0f;
    const float channelHeight = channelTopY - channelBottomY;
    const float channelCenterY = (channelTopY + channelBottomY) * 0.5f;

    glm::mat4 leftChannel = composeModel(wadhuTransform, glm::vec3(-18.5f, channelCenterY, -25.5f), glm::vec3(0.8f, channelHeight, 48.6f));
    myCube->draw(lightingShader, leftChannel, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw left-side water flow channel.

    // Extend front/back channels slightly toward the left wall so all three channels intersect
    glm::mat4 frontChannel = composeModel(wadhuTransform, glm::vec3(-0.6f, channelCenterY, -1.25f), glm::vec3(36.2f, channelHeight, 0.4f));
    myCube->draw(lightingShader, frontChannel, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw front water flow channel.

    glm::mat4 backChannel = composeModel(wadhuTransform, glm::vec3(-0.6f, channelCenterY, -49.75f), glm::vec3(36.2f, channelHeight, 0.4f));
    myCube->draw(lightingShader, backChannel, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw back water flow channel.

    // --- LEFT WADHU PLACES ---
    glBindTexture(GL_TEXTURE_2D, texWahdou);
    lightingShader.setVec2("uvTiling", 4.0f, 4.0f);
    const float wadhuWallBottomY = 0.0f;
    const float wadhuWallTopY = kMosqueRoofY;
    const float wadhuWallHeight = wadhuWallTopY - wadhuWallBottomY;
    const float wadhuWallCenterY = wadhuWallBottomY + (wadhuWallHeight * 0.5f);

    // Left wadhu place #1
    {
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(-19.5f, wadhuWallCenterY, -25.5f), glm::vec3(1.0f, wadhuWallHeight, 51.0f));
        myCube->draw(lightingShader, m, wallColor); // Draw wadhu left perimeter wall.
    }
    // Left wadhu place #2
    {
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(0.0f, wadhuWallCenterY, -50.5f), glm::vec3(40.0f, wadhuWallHeight, 1.0f));
        myCube->draw(lightingShader, m, wallColor); // Draw wadhu back perimeter wall.
    }
    // Left wadhu place #3
    {
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(0.0f, wadhuWallCenterY, -0.5f), glm::vec3(40.0f, wadhuWallHeight, 1.0f));
        myCube->draw(lightingShader, m, wallColor); // Draw wadhu front perimeter wall.
    }

    // Tile-like top slab
    {
        glBindTexture(GL_TEXTURE_2D, texWahdou);
        lightingShader.setVec2("uvTiling", 8.0f, 4.0f);
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(0.0f, kMosqueRoofY, -25.5f), glm::vec3(40.0f, 0.5f, 51.0f));
        myCube->draw(lightingShader, m, tileColor); // Draw wadhu top slab/cap.
    }

    // Wadu seating boxes (places to sit) along front, back, and left side wall

    // Along Left Wall (x = -16.0f)
    for (int i = 0; i < 4; ++i)
    {
        float sz = -10.0f - (i * 10.0f); // Spaced along Z: -10, -20, -30, -40
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(-16.0f, 4.0f, sz), glm::vec3(3.0f, 2.0f, 3.0f));
        myCube->draw(lightingShader, m, tileColor); // Draw one left-wall seating block.
    }

    // Along Front Wall (z = -3.5f)
    for (int i = 0; i < 3; ++i)
    {
        float sx = -6.0f + (i * 10.0f); // Spaced along X: -6, 4, 14
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(sx, 4.0f, -3.5f), glm::vec3(3.0f, 2.0f, 3.0f));
        myCube->draw(lightingShader, m, tileColor); // Draw one front-wall seating block.
    }

    // Along Back Wall (z = -47.5f)
    for (int i = 0; i < 3; ++i)
    {
        float sx = -6.0f + (i * 10.0f); // Spaced along X: -6, 4, 14
        glm::mat4 m = composeModel(wadhuTransform, glm::vec3(sx, 4.0f, -47.5f), glm::vec3(3.0f, 2.0f, 3.0f));
        myCube->draw(lightingShader, m, tileColor); // Draw one back-wall seating block.
    }

    // Wadhu faucets (simple: horizontal cylinder out of wall + vertical cylinder down)
    {
        // Use the light cube shader so the faucet looks like solid metal (no texture).
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        const float pipeY = 5.45f;

        // Modern faucet drawing lambda
        auto drawFancyFaucet = [&](glm::vec3 bPos, glm::vec3 tPos)
        {
            glm::vec3 horizontalDir = glm::normalize(tPos - bPos);
            float horizLen = glm::length(tPos - bPos);

            glm::vec3 UP(0.0f, 1.0f, 0.0f);
            glm::vec3 RIGHT = glm::normalize(glm::cross(UP, horizontalDir));

            glm::mat4 localBasis(1.0f);
            localBasis[0] = glm::vec4(RIGHT, 0.0f);
            localBasis[1] = glm::vec4(UP, 0.0f);
            localBasis[2] = glm::vec4(horizontalDir, 0.0f); // Forward
            localBasis[3] = glm::vec4(bPos, 1.0f);

            glm::mat4 modelBase = wadhuTransform * localBasis;

            // Stylish metallic colors
            glm::vec3 brightChrome(0.85f, 0.90f, 0.95f);
            glm::vec3 darkChrome(0.55f, 0.60f, 0.65f);

            lightCubeShader.setVec3("lightColor", brightChrome);

            // 1. Base thick horizontal sleeve
            float sleeveLen = horizLen * 0.30f;
            glm::mat4 mSleeve = modelBase;
            mSleeve = glm::translate(mSleeve, glm::vec3(0.0f, 0.0f, sleeveLen * 0.5f));
            mSleeve = glm::rotate(mSleeve, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            mSleeve = glm::scale(mSleeve, glm::vec3(0.07f, sleeveLen * 0.5f, 0.07f));
            myCylinder->draw(lightCubeShader, mSleeve, brightChrome); // Draw faucet base sleeve.

            // 2. Main horizontal pipe (thinner) extending to tip
            glm::mat4 mMain = modelBase;
            mMain = glm::translate(mMain, glm::vec3(0.0f, 0.0f, horizLen * 0.5f));
            mMain = glm::rotate(mMain, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            mMain = glm::scale(mMain, glm::vec3(0.05f, horizLen * 0.5f, 0.05f));
            myCylinder->draw(lightCubeShader, mMain, brightChrome); // Draw faucet main horizontal pipe.

            // 3. Vertical drop pipe at the spout tip (points toward -Y)
            // Keep it short like a real faucet neck.
            float dropPipeLen = 0.15f;
            float dropOffsetUp = 0.035f;
            glm::mat4 mDrop = modelBase;
            mDrop = glm::translate(mDrop, glm::vec3(0.0f, -dropPipeLen * 0.5f + dropOffsetUp, horizLen));
            mDrop = glm::scale(mDrop, glm::vec3(0.04f, dropPipeLen * 0.5f, 0.04f));
            myCylinder->draw(lightCubeShader, mDrop, brightChrome); // Draw faucet vertical drop pipe.

            // 4. Nozzle/aerator
            lightCubeShader.setVec3("lightColor", darkChrome);
            glm::mat4 mAerator = modelBase;
            mAerator = glm::translate(mAerator, glm::vec3(0.0f, -dropPipeLen - 0.02f + dropOffsetUp, horizLen));
            mAerator = glm::scale(mAerator, glm::vec3(0.05f, 0.02f, 0.05f));
            myCylinder->draw(lightCubeShader, mAerator, darkChrome); // Draw faucet aerator/nozzle tip.

            // 5. Lever handle mechanism closer to the nozzle
            float handleZ = horizLen * 0.75f;

            // Handle base cylinder (replaces the weird sphere)
            glm::mat4 mHBase = modelBase;
            mHBase = glm::translate(mHBase, glm::vec3(0.0f, 0.07f, handleZ));
            mHBase = glm::scale(mHBase, glm::vec3(0.05f, 0.02f, 0.05f));
            myCylinder->draw(lightCubeShader, mHBase, darkChrome); // Draw faucet handle base cylinder.

            // Handle vertical pivot
            lightCubeShader.setVec3("lightColor", brightChrome);
            glm::mat4 mHPivot = modelBase;
            mHPivot = glm::translate(mHPivot, glm::vec3(0.0f, 0.11f, handleZ));
            mHPivot = glm::scale(mHPivot, glm::vec3(0.02f, 0.02f, 0.02f));
            myCylinder->draw(lightCubeShader, mHPivot, brightChrome); // Draw faucet handle pivot post.

            // Handle lever arm (pointing forward and rotating on Y axis)
            lightCubeShader.setVec3("lightColor", darkChrome);
            glm::mat4 mLever = modelBase;
            float handleYaw = faucetWaterOn ? 45.0f : 0.0f; // rotate handle horizontally when on
            mLever = glm::translate(mLever, glm::vec3(0.0f, 0.13f, handleZ));
            mLever = glm::rotate(mLever, glm::radians(handleYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            mLever = glm::translate(mLever, glm::vec3(0.0f, 0.0f, 0.04f));
            mLever = glm::scale(mLever, glm::vec3(0.045f, 0.01f, 0.065f));
            myCube->draw(lightCubeShader, mLever, darkChrome); // Draw faucet lever handle.
        };

        // LEFT wall faucets (water channel is at x = -18.5)
        for (int i = 0; i < 4; ++i)
        {
            float zSeat = -10.0f - (i * 10.0f);
            const float xTip = -18.38f;
            const float xBase = -19.08f;
            drawFancyFaucet(glm::vec3(xBase, pipeY, zSeat), glm::vec3(xTip, pipeY, zSeat));
        }

        // FRONT wall faucets (water channel is at z = -1.25)
        for (int i = 0; i < 3; ++i)
        {
            float xSeat = -6.0f + (i * 10.0f);
            const float zTip = -1.40f;
            const float zBase = -0.95f;
            drawFancyFaucet(glm::vec3(xSeat, pipeY, zBase), glm::vec3(xSeat, pipeY, zTip));
        }

        // BACK wall faucets (water channel is at z = -49.75)
        for (int i = 0; i < 3; ++i)
        {
            float xSeat = -6.0f + (i * 10.0f);
            const float zTip = -49.60f;
            const float zBase = -50.05f;
            drawFancyFaucet(glm::vec3(xSeat, pipeY, zBase), glm::vec3(xSeat, pipeY, zTip));
        }
    }

    // Wadhu lamp hemisphere.
    // Important: make it relative to `wadhuTransform` so moving Wadhu only needs changing the transform at the call site.
    glm::vec3 wadhuLampColor = pointLightsOn ? (glm::vec3(0.75f, 0.80f, 1.0f) * 1.25f) : glm::vec3(0.25f, 0.25f, 0.25f);

    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);

    glm::mat4 mWadhu = wadhuTransform;
    mWadhu = glm::translate(mWadhu, glm::vec3(0.0f, kMosqueRoofY, -25.5f));
    mWadhu = glm::rotate(mWadhu, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mWadhu = glm::scale(mWadhu, glm::vec3(1.55f, 1.55f, 1.55f));

    lightCubeShader.setMat4("model", mWadhu);
    lightCubeShader.setVec3("lightColor", wadhuLampColor);
    mySphere->drawHemisphere(lightCubeShader, mWadhu, wadhuLampColor); // Draw wadhu hanging lamp shade.
}

// This function draws the cylindrical pillars for both the front and right verandas
// of the mosque.
static void drawVerandaPillars(
    Shader &lightingShader,
    Cylinder *myCylinder,
    GLuint texWall,
    const glm::mat4 &mosqueRoot,
    float mosqueZ,
    const glm::vec3 &pillarColor)
{
    glBindTexture(GL_TEXTURE_2D, texWall);
    lightingShader.setVec2("uvTiling", 1.0f, 4.0f);

    // Cylinder is scaled from center with half-height in Y,
    // so these values make pillars end exactly at the roof level.
    float pillarBottomY = 3.0f;
    float pillarTopY = kMosqueRoofY;
    float pillarYCenter = (pillarBottomY + pillarTopY) * 0.5f;
    float pillarHeight = (pillarTopY - pillarBottomY) * 0.5f;

    std::vector<float> frontPillarXs = {-30.0f, 5.0f, 40.0f, 75.0f, 110.0f, 145.0f};
    std::vector<float> frontPillarZs = {mosqueZ + 25.0f, mosqueZ + 45.0f, mosqueZ + 65.0f};

    for (float px : frontPillarXs)
    {
        for (float pz : frontPillarZs)
        {
            glm::mat4 m = composeModel(mosqueRoot,
                                       glm::vec3(px - 40.0f, pillarYCenter, pz - (mosqueZ - 30.0f)),
                                       glm::vec3(1.2f, pillarHeight, 1.2f));
            myCylinder->draw(lightingShader, m, pillarColor); // Draw one front veranda pillar.
        }
    }

    float rightPillarX = 145.0f;
    std::vector<float> rightPillarZs = {mosqueZ - 75.0f, mosqueZ - 45.0f, mosqueZ - 15.0f};

    for (float pz : rightPillarZs)
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(rightPillarX - 40.0f, pillarYCenter, pz - (mosqueZ - 30.0f)),
                                   glm::vec3(1.2f, pillarHeight, 1.2f));
        myCylinder->draw(lightingShader, m, pillarColor); // Draw one right veranda pillar.
    }
}

// This function constructs and draws the two tall minarets flanking the mosque.
// It draws their pedestals, layered shafts, balconies with railings, the top
// lantern layer, domes, and the decorative golden finials at the peaks.
static void drawMinarets(
    Shader &lightingShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    Cone *myCone,
    GLuint texWall,
    const glm::mat4 &mosqueRoot,
    const glm::vec3 &minaretColor,
    const glm::vec3 &roofColor,
    const glm::vec3 &wallColor,
    const glm::vec3 &domeColor,
    const glm::vec3 &goldColor)
{
    glBindTexture(GL_TEXTURE_2D, texWall);
    lightingShader.setVec2("uvTiling", 2.0f, 4.0f);

    float mZ = -54.0f; // Moved closer to the main back wall (edge is at -49.0f)
    (void)mZ;

    for (float signX : {-1.0f, 1.0f})
    {
        float mX = signX * 86.0f; // Left and right symmetric minarets
        float mZLocal = (signX > 0.0f) ? -56.0f : -54.0f;

        // Scale the entire minaret hierarchically to make it much larger
        glm::mat4 minaretRoot = glm::translate(mosqueRoot, glm::vec3(mX, 0.0f, mZLocal));
        minaretRoot = glm::scale(minaretRoot, glm::vec3(1.6f, 1.6f, 1.6f));

        // 1. Base (Square Pedestal)
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(8.0f, 12.0f, 8.0f));
            myCube->draw(lightingShader, m, minaretColor); // Draw minaret pedestal base block.
        }

        // 2. Transition Section (Cylindrical)
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(3.2f, 6.0f, 3.2f));
            myCylinder->draw(lightingShader, m, minaretColor); // Draw minaret transition section.
        }

        // 3. Lower Shaft
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 26.0f, 0.0f), glm::vec3(2.8f, 16.0f, 2.8f));
            myCylinder->draw(lightingShader, m, minaretColor); // Draw minaret lower shaft.
        }

        // 4. Balcony 1 Floor
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 34.0f, 0.0f), glm::vec3(4.0f, 0.6f, 4.0f));
            myCylinder->draw(lightingShader, m, roofColor); // Draw minaret first balcony floor.
        }

        // 5. Balcony 1 Railing
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 35.3f, 0.0f), glm::vec3(3.9f, 2.0f, 3.9f));
            myCylinder->draw(lightingShader, m, wallColor); // Draw minaret first balcony railing.
        }

        // 6. Upper Shaft
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 42.0f, 0.0f), glm::vec3(2.4f, 13.4f, 2.4f));
            myCylinder->draw(lightingShader, m, minaretColor); // Draw minaret upper shaft.
        }

        // 7. Balcony 2 Floor
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 48.7f, 0.0f), glm::vec3(3.4f, 0.5f, 3.4f));
            myCylinder->draw(lightingShader, m, roofColor); // Draw minaret second balcony floor.
        }

        // 8. Balcony 2 Railing
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 49.8f, 0.0f), glm::vec3(3.3f, 1.6f, 3.3f));
            myCylinder->draw(lightingShader, m, wallColor); // Draw minaret second balcony railing.
        }

        // 9. Top Gallery / Lantern
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 54.0f, 0.0f), glm::vec3(1.8f, 6.8f, 1.8f));
            myCylinder->draw(lightingShader, m, minaretColor); // Draw minaret lantern/gallery body.
        }

        // 10. Minaret Dome / Cupola
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 57.4f, 0.0f), glm::vec3(2.2f, 2.2f, 2.2f));
            mySphere->drawHemisphere(lightingShader, m, domeColor); // Draw minaret cupola dome.
        }

        // 10b. Minaret cone ornament
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 65.2f, 0.0f), glm::vec3(0.55f, 0.95f, 0.55f));
            myCone->draw(lightingShader, m, goldColor); // Draw minaret top cone ornament.
        }

        // 11. Finial Pillar (Gold tip base)
        {
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 60.5f, 0.0f), glm::vec3(0.2f, 4.0f, 0.2f));
            myCylinder->draw(lightingShader, m, goldColor); // Draw minaret finial stem.
        }

        // 12. Finial Spheres
        {
            // Lower bulb
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
            mySphere->draw(lightingShader, m, goldColor); // Draw minaret finial lower sphere.
        }
        {
            // Middle bulb
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 61.2f, 0.0f), glm::vec3(0.4f, 0.4f, 0.4f));
            mySphere->draw(lightingShader, m, goldColor); // Draw minaret finial middle sphere.
        }
        {
            // Top tiny bulb
            glm::mat4 m = composeModel(minaretRoot, glm::vec3(0.0f, 62.4f, 0.0f), glm::vec3(0.25f, 0.25f, 0.25f));
            mySphere->draw(lightingShader, m, goldColor); // Draw minaret finial top sphere.
        }
    }
}

// This function draws the exterior walkways leading to the mosque. It includes
// the main path, side paths, the staircases leading up to the mosque verandas,
// and the lamp posts lining the sides of the walkways.
static void drawWalkwayAndStairs(
    Shader &lightingShader,
    Cube *myCube,
    Cylinder *myCylinder,
    GLuint texRoad,
    GLuint texWall,
    GLuint texFloor,
    const glm::mat4 &sceneRoot,
    const glm::mat4 &mosqueRoot,
    int numLampPairs,
    float lampFirst,
    float lampStep,
    float lampXOffset,
    const glm::vec3 &tileColor,
    int numRightLampPairs,
    float rightLampFirst,
    float rightLampStep,
    float rightLampZOffset)
{
    glm::vec3 pathColor(0.82f, 1.0f, 0.82f);
    glm::vec3 lampPostColor(1.0f, 1.0f, 1.0f);

    // Main walkway path
    float pathStartZ = kFrontRoadStartZ;
    float pathEndZ = kFrontRoadEndZ;
    float pathLen = pathEndZ - pathStartZ;
    glm::mat4 pathRoot = glm::translate(sceneRoot, glm::vec3(kFrontRoadCenterX, 0.0f, pathStartZ));

    glBindTexture(GL_TEXTURE_2D, texRoad);
    lightingShader.setVec2("uvTiling", 2.0f, 24.0f);
    lightingShader.setFloat("materialTexBlend", 0.5f);
    glm::mat4 m = composeModel(pathRoot, glm::vec3(0.0f, kRoadSurfaceY, pathLen * 0.5f), glm::vec3(kRoadWidth, kRoadThickness, pathLen));
    myCube->draw(lightingShader, m, pathColor); // Draw the main front walkway/road slab.
    lightingShader.setFloat("materialTexBlend", 0.0f);

    for (int i = 0; i < numLampPairs; ++i)
    {
        float lz = lampFirst + i * lampStep;
        glm::mat4 lampPairRoot = glm::translate(pathRoot, glm::vec3(0.0f, 0.0f, lz - pathStartZ));

        for (float lx : {-lampXOffset, lampXOffset})
        {
            glBindTexture(GL_TEXTURE_2D, texWall);
            lightingShader.setVec2("uvTiling", 1.0f, 4.0f);
            glm::mat4 lm = composeModel(lampPairRoot, glm::vec3(lx, 3.5f, 0.0f), glm::vec3(0.25f, 7.0f, 0.25f));
            myCylinder->draw(lightingShader, lm, lampPostColor); // Draw one front-road lamp post shaft.
        }
    }

    // Mosque access stairs from main road to front veranda
    glBindTexture(GL_TEXTURE_2D, texRoad);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    lightingShader.setFloat("materialTexBlend", 0.5f);
    const int stepCount = 4;
    const float stepHeight = 0.75f;
    const float stepDepth = 2.0f;
    const float stepWidth = 14.0f;
    const float frontVerandaEdgeZ = 100.0f; // local to mosqueRoot

    for (int i = 0; i < stepCount; ++i)
    {
        float stepY = (stepCount - i) * stepHeight;
        float stepCenterY = stepY * 0.5f;
        float stepCenterZ = frontVerandaEdgeZ + (i + 0.5f) * stepDepth;
        glm::mat4 sm = composeModel(mosqueRoot, glm::vec3(0.0f, stepCenterY, stepCenterZ), glm::vec3(stepWidth, stepY, stepDepth));
        myCube->draw(lightingShader, sm, tileColor); // Draw one front stair step block.
    }
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // Stair access to right veranda (outside right edge)
    glBindTexture(GL_TEXTURE_2D, texRoad);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    lightingShader.setFloat("materialTexBlend", 0.5f);
    const float stepWidthZ = 14.0f;
    const float rightVerandaEdgeX = 110.0f; // local to mosqueRoot

    for (int i = 0; i < stepCount; ++i)
    {
        float stepY = (stepCount - i) * stepHeight;
        float stepCenterY = stepY * 0.5f;
        float stepCenterX = rightVerandaEdgeX + (i + 0.5f) * stepDepth;
        glm::mat4 sm = composeModel(mosqueRoot, glm::vec3(stepCenterX, stepCenterY, 0.0f), glm::vec3(stepDepth, stepY, stepWidthZ));
        myCube->draw(lightingShader, sm, tileColor); // Draw one right-side stair step block.
    }
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // ====== Right road (extending from right veranda stairs outward in +X) ======
    // The right veranda stairs end at mosqueRoot local X ~= 118.
    // In world coords: mosqueRoot translates by (40, 0, mosqueZ-30), so local X=118 => world X=158.
    // Road extends from world X=160 to the ground max X boundary.
    float rightRoadStartX = kRightRoadStartX;
    float rightRoadEndX = kRightRoadEndX;
    float rightRoadLen = rightRoadEndX - rightRoadStartX;
    float rightRoadCenterZ = kRightRoadCenterZ;

    glBindTexture(GL_TEXTURE_2D, texRoad);
    lightingShader.setVec2("uvTiling", 24.0f, 2.0f);
    lightingShader.setFloat("materialTexBlend", 0.5f);
    {
        glm::mat4 rightRoadModel = composeModel(sceneRoot,
                                                glm::vec3(rightRoadStartX + rightRoadLen * 0.5f - 2.0f, kRoadSurfaceY, rightRoadCenterZ),
                                                glm::vec3(rightRoadLen, kRoadThickness, kRoadWidth));
        myCube->draw(lightingShader, rightRoadModel, pathColor); // Draw the right-side road extension.
    }
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // Right road lamp posts
    for (int i = 0; i < numRightLampPairs; ++i)
    {
        float lx = rightLampFirst + i * rightLampStep;

        for (float lz : {-rightLampZOffset, rightLampZOffset})
        {
            glBindTexture(GL_TEXTURE_2D, texWall);
            lightingShader.setVec2("uvTiling", 1.0f, 4.0f);
            glm::mat4 lm = composeModel(sceneRoot,
                                        glm::vec3(lx, 3.5f, rightRoadCenterZ + lz),
                                        glm::vec3(0.25f, 7.0f, 0.25f));
            myCylinder->draw(lightingShader, lm, lampPostColor); // Draw one right-road lamp post shaft.
        }
    }
}

// This function draws a rectangular decorative water pond, including its stone basin,
// rim walls, and the flat water surface layer.
static void drawPond(
    Shader &lightingShader,
    Cube *myCube,
    const glm::mat4 &sceneRoot,
    glm::vec3 pondPos,
    float pondW,
    float pondD,
    float pondWallH,
    float pondWallT,
    const glm::vec3 &waterColor,
    const glm::vec3 &rimColor)
{
    glm::mat4 pondRoot = glm::translate(sceneRoot, pondPos);

    // Pond basin
    glBindTexture(GL_TEXTURE_2D, texPool);
    lightingShader.setVec2("uvTiling", 4.0f, 6.0f);
    glm::mat4 m = composeModel(pondRoot, glm::vec3(0.0f, -0.08f, 0.0f), glm::vec3(pondW + 0.2f, 0.06f, pondD + 0.2f));
    myCube->draw(lightingShader, m, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw pond basin floor slab.

    // Water surface
    glBindTexture(GL_TEXTURE_2D, texPool);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    glm::mat4 wm = composeModel(pondRoot, glm::vec3(0.0f, 0.02f, 0.0f), glm::vec3(pondW - pondWallT * 0.5f, 0.06f, pondD - pondWallT * 0.5f));
    myCube->draw(lightingShader, wm, waterColor); // Draw pond water surface.

    // Pond rim walls (4 sides)
    glBindTexture(GL_TEXTURE_2D, texPool);
    glm::mat4 rm1 = composeModel(pondRoot, glm::vec3(0.0f, pondWallH * 0.5f, pondD * 0.5f), glm::vec3(pondW + pondWallT, pondWallH, pondWallT));
    myCube->draw(lightingShader, rm1, rimColor); // Draw pond front rim wall.

    glm::mat4 rm2 = composeModel(pondRoot, glm::vec3(0.0f, pondWallH * 0.5f, -pondD * 0.5f), glm::vec3(pondW + pondWallT, pondWallH, pondWallT));
    myCube->draw(lightingShader, rm2, rimColor); // Draw pond back rim wall.

    glm::mat4 rm3 = composeModel(pondRoot, glm::vec3(-pondW * 0.5f, pondWallH * 0.5f, 0.0f), glm::vec3(pondWallT, pondWallH, pondD + pondWallT));
    myCube->draw(lightingShader, rm3, rimColor); // Draw pond left rim wall.

    glm::mat4 rm4 = composeModel(pondRoot, glm::vec3(pondW * 0.5f, pondWallH * 0.5f, 0.0f), glm::vec3(pondWallT, pondWallH, pondD + pondWallT));
    myCube->draw(lightingShader, rm4, rimColor); // Draw pond right rim wall.
}

// This function draws the glowing light bulbs / glass lamp heads sitting atop the
// lampposts along the exterior roads, rendering them as emissive spheres.
static void drawRoadLampHeadVisuals(
    Shader &lightCubeShader,
    Sphere *mySphere,
    const glm::mat4 &projection,
    const glm::mat4 &view,
    const glm::mat4 &sceneRoot,
    const glm::vec3 &lampLightColor,
    int numLampPairs,
    float lampFirst,
    float lampStep,
    float lampXOffset,
    int numRightLampPairs,
    float rightLampFirst,
    float rightLampStep,
    float rightLampZOffset)
{
    // Light heads live on the walkway path (same positions as the real point lights).
    float pathStartZ = kFrontRoadStartZ;
    glm::mat4 pathRoot = glm::translate(sceneRoot, glm::vec3(40.0f, 0.0f, pathStartZ));

    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);

    // Draw lamp heads (point light sources) as emissive spheres
    for (int i = 0; i < numLampPairs; ++i)
    {
        float lz = lampFirst + i * lampStep;
        glm::mat4 lampPairRoot = glm::translate(pathRoot, glm::vec3(0.0f, 0.0f, lz - pathStartZ));
        float lampOffsets[2] = {-lampXOffset, lampXOffset};

        for (int side = 0; side < 2; ++side)
        {
            glm::mat4 m = composeModel(lampPairRoot, glm::vec3(lampOffsets[side], 10.0f, 0.0f), glm::vec3(0.8f, 0.8f, 0.8f));
            lightCubeShader.setMat4("model", m);
            glm::vec3 lampColor = pointLightsOn ? lampLightColor : glm::vec3(0.3f, 0.3f, 0.3f);
            lightCubeShader.setVec3("lightColor", lampColor);
            mySphere->draw(lightCubeShader, m, lampColor); // Draw one front-road lamp head (light visual).
        }
    }

    // Right road lamp heads
    float rightRoadCenterZ = kRightRoadCenterZ;
    for (int i = 0; i < numRightLampPairs; ++i)
    {
        float lx = rightLampFirst + i * rightLampStep;
        float lampOffsets[2] = {-rightLampZOffset, rightLampZOffset};

        for (int side = 0; side < 2; ++side)
        {
            glm::mat4 m = composeModel(sceneRoot, glm::vec3(lx, 10.0f, rightRoadCenterZ + lampOffsets[side]), glm::vec3(0.8f, 0.8f, 0.8f));
            lightCubeShader.setMat4("model", m);
            glm::vec3 lampColor = pointLightsOn ? lampLightColor : glm::vec3(0.3f, 0.3f, 0.3f);
            lightCubeShader.setVec3("lightColor", lampColor);
            mySphere->draw(lightCubeShader, m, lampColor); // Draw one right-road lamp head (light visual).
        }
    }

    // Road/walkway lamp heads only.
    // Mosque/Veranda light visuals are handled by their own stage-specific draw calls.
}

// This function draws the large flat grass ground plane that acts as the
// foundational terrain for the entire outdoor scene.
static void drawGround(
    Shader &lightingShader,
    Cube *myCube,
    const glm::mat4 &sceneRoot)
{
    // =====================================================
    // 1. GROUND (grass)
    // =====================================================
    {
        glBindTexture(GL_TEXTURE_2D, texGrass);
        lightingShader.setVec2("uvTiling", 40.0f, 40.0f);
        glm::mat4 m = composeModel(sceneRoot, glm::vec3(kGroundCenterX, kGroundCenterY, kGroundCenterZ), glm::vec3(kGroundSizeX, kGroundSizeY, kGroundSizeZ));
        myCube->draw(lightingShader, m, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw the large grass ground plane.
    }

    // Ground-only: mosque main base + veranda bases + mihrab base are drawn elsewhere.
}

// =====================================================
// FRACTAL TREE SYSTEM
// =====================================================

// Generates a deterministic pseudo-random float value between 0.0 and 1.0
// based on a mutable seed reference, used for procedurally generating fractal trees.
static float treeRand(unsigned int &seed)
{
    seed = seed * 1103515245u + 12345u;
    return (float)((seed >> 16) & 0x7FFF) / 32768.0f;
}

// Recursive fractal branch: draws a cylinder segment then spawns children or leaves.
// It creates the tree hierarchy by drawing cylinders for branches and spawning
// terminal spheres acting as leaf clusters at the maximum branching depth.
static void drawFractalBranch(
    Shader &lightingShader,
    Cylinder *cyl,
    Sphere *sph,
    const glm::mat4 &baseTransform,
    float length,
    float radius,
    int depth,
    int maxDepth,
    unsigned int seed,
    const glm::vec3 &barkColor,
    const glm::vec3 &leafColor)
{
    // --- Draw this branch segment ---
    {
        glBindTexture(GL_TEXTURE_2D, texWood);
        lightingShader.setVec2("uvTiling", 1.0f, 3.0f);
        lightingShader.setFloat("materialTexBlend", 0.35f);

        glm::mat4 m = baseTransform;
        m = glm::translate(m, glm::vec3(0.0f, length * 0.5f, 0.0f));
        m = glm::scale(m, glm::vec3(radius, length * 0.5f, radius));

        float t = (float)depth / (float)maxDepth;
        glm::vec3 branchCol = glm::mix(barkColor, barkColor * 0.72f, t);
        cyl->draw(lightingShader, m, branchCol);
    }

    // Tip transform (top of this branch)
    glm::mat4 tipTf = glm::translate(baseTransform, glm::vec3(0.0f, length, 0.0f));

    // --- Terminal depth: draw leaf clusters ---
    if (depth >= maxDepth)
    {
        glBindTexture(GL_TEXTURE_2D, texGrass);
        lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
        lightingShader.setFloat("materialTexBlend", 0.78f);

        float ls = length * 0.85f;
        unsigned int lseed = seed;
        float cv = treeRand(lseed) * 0.06f - 0.03f;

        // Single leaf sphere per branch tip for a sparser canopy
        {
            glm::mat4 lm = glm::translate(tipTf, glm::vec3(0.0f, ls * 0.15f, 0.0f));
            lm = glm::scale(lm, glm::vec3(ls, ls * 0.6f, ls));
            sph->draw(lightingShader, lm, leafColor + glm::vec3(cv, cv * 0.5f, cv * 0.3f));
        }
        lightingShader.setFloat("materialTexBlend", 0.0f);
        return;
    }

    // --- Spawn child branches ---
    int numLateral = 2 + (seed % 2); // 2 or 3 lateral branches
    float childLen = length * 0.68f;
    float childRad = radius * 0.58f;
    unsigned int bseed = seed;

    // Continuation branch (roughly upward with small jitter)
    {
        float jx = (treeRand(bseed) - 0.5f) * 15.0f;
        float jz = (treeRand(bseed) - 0.5f) * 15.0f;
        glm::mat4 ct = tipTf;
        ct = glm::rotate(ct, glm::radians(jx), glm::vec3(0.0f, 0.0f, 1.0f));
        ct = glm::rotate(ct, glm::radians(jz), glm::vec3(1.0f, 0.0f, 0.0f));
        drawFractalBranch(lightingShader, cyl, sph, ct,
                          childLen * 1.05f, childRad * 1.1f,
                          depth + 1, maxDepth, bseed * 31u + 7u,
                          barkColor, leafColor);
    }

    // Lateral branches at various azimuths
    for (int i = 0; i < numLateral; ++i)
    {
        float azimuth = ((float)i / (float)numLateral) * 360.0f + treeRand(bseed) * 50.0f;
        float elevation = 25.0f + treeRand(bseed) * 25.0f;
        glm::mat4 ct = tipTf;
        ct = glm::rotate(ct, glm::radians(azimuth), glm::vec3(0.0f, 1.0f, 0.0f));
        ct = glm::rotate(ct, glm::radians(elevation), glm::vec3(0.0f, 0.0f, 1.0f));
        unsigned int cs = seed * 31u + (unsigned int)i * 17u + (unsigned int)depth * 7u;
        drawFractalBranch(lightingShader, cyl, sph, ct,
                          childLen, childRad,
                          depth + 1, maxDepth, cs,
                          barkColor, leafColor);
    }
}

// Initializes and draws a complete procedural fractal tree at a specified world
// position by calling the recursive branch drawing function from the base trunk.
static void drawFractalTree(
    Shader &lightingShader,
    Cylinder *cyl,
    Sphere *sph,
    const glm::mat4 &sceneRoot,
    const glm::vec3 &position,
    float trunkHeight,
    float trunkRadius,
    int maxDepth,
    unsigned int seed,
    const glm::vec3 &barkColor,
    const glm::vec3 &leafColor)
{
    glm::mat4 treeBase = glm::translate(sceneRoot, position);
    drawFractalBranch(lightingShader, cyl, sph, treeBase,
                      trunkHeight, trunkRadius,
                      0, maxDepth, seed,
                      barkColor, leafColor);
}

// Places and renders multiple uniquely seeded fractal trees at predefined positions
// around the grass grounds of the mosque scene.
static void drawTrees(
    Shader &lightingShader,
    Cylinder *myCylinder,
    Sphere *mySphere,
    const glm::mat4 &sceneRoot)
{
    glm::vec3 bark1(0.40f, 0.26f, 0.13f);
    glm::vec3 bark2(0.50f, 0.33f, 0.18f);
    glm::vec3 leaf1(0.13f, 0.50f, 0.10f);
    glm::vec3 leaf2(0.18f, 0.58f, 0.15f);
    glm::vec3 leaf3(0.22f, 0.52f, 0.12f);
    glm::vec3 leaf4(0.15f, 0.45f, 0.20f);

    struct TreeSpec
    {
        glm::vec3 pos;
        float h, r;
        int d;
        unsigned int s;
        glm::vec3 bk, lf;
    };
    TreeSpec trees[] = {
        // Left side of front road (closer to mosque)
        {{15.0f, 0.0f, 30.0f}, 8.0f, 0.7f, 3, 42u, bark1, leaf1},
        {{10.0f, 0.0f, 130.0f}, 10.0f, 0.9f, 4, 73u, bark2, leaf2},
        {{18.0f, 0.0f, 190.0f}, 9.0f, 0.8f, 3, 101u, bark1, leaf3},
        // Right side of front road (closer to mosque)
        {{68.0f, 0.0f, 30.0f}, 9.0f, 0.8f, 3, 55u, bark2, leaf2},
        {{72.0f, 0.0f, 130.0f}, 11.0f, 1.0f, 4, 89u, bark1, leaf1},
        {{65.0f, 0.0f, 190.0f}, 8.5f, 0.75f, 3, 127u, bark2, leaf4},
        // Left side of right road (lower Z side)
        {{190.0f, 0.0f, -135.0f}, 9.0f, 0.8f, 3, 301u, bark1, leaf3},
        {{235.0f, 0.0f, -135.0f}, 10.0f, 0.9f, 4, 313u, bark2, leaf1},
        {{280.0f, 0.0f, -135.0f}, 8.5f, 0.75f, 3, 327u, bark1, leaf4},
        // Right side of right road (upper Z side)
        {{190.0f, 0.0f, -85.0f}, 10.0f, 0.9f, 4, 341u, bark2, leaf2},
        {{235.0f, 0.0f, -85.0f}, 8.0f, 0.7f, 3, 353u, bark1, leaf1},
        {{280.0f, 0.0f, -85.0f}, 11.0f, 1.0f, 4, 367u, bark2, leaf3},
    };

    int count = sizeof(trees) / sizeof(trees[0]);
    for (int i = 0; i < count; ++i)
    {
        drawFractalTree(lightingShader, myCylinder, mySphere,
                        sceneRoot, trees[i].pos, trees[i].h, trees[i].r,
                        trees[i].d, trees[i].s,
                        trees[i].bk, trees[i].lf);
    }
    lightingShader.setFloat("materialTexBlend", 0.0f);
}

// This function draws the main roof slab of the mosque, the large central dome,
// the golden cone ornament on top of the dome, and the array of rotating
// ceiling fans attached inside the main mosque hall.
static void drawMosqueRoofDomeAndFans(
    Shader &lightingShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    Cone *myCone,
    GLuint texWall,
    GLuint texDome,
    const glm::mat4 &mosqueRoot,
    const glm::vec3 &roofColor,
    const glm::vec3 &domeColor,
    const glm::vec3 &goldColor)
{
    // 6. Top (Roof slab) + main dome + ceiling fans
    {
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 10.0f, 10.0f);
        float roofTotalW = 160.0f;
        float roofTotalD = 100.0f;
        // Compute opening to match dome footprint. Adjust padding/offsets if needed.
        float domeScale = 22.0f;         // matches dome scale used below (diameter)
        float domeOpeningPadding = 0.6f; // increase this if opening looks too small
        float domeOpeningRadius = (domeScale * 0.95f) + domeOpeningPadding;
        float domeOpeningOffsetX = 0.0f;           // tweak to nudge X center if necessary
        float domeOpeningOffsetZ = 0.0f;           // tweak to nudge Z center if necessary
        float domeOpeningY = kMosqueRoofY + 0.01f; // place mask at roof plane (slightly above to avoid z-fighting)

        {
            // Cut a circular opening in the roof directly below the main dome.
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xFF);
            glClear(GL_STENCIL_BUFFER_BIT);

            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            glm::mat4 domeOpeningMask = composeModel(mosqueRoot,
                                                     glm::vec3(domeOpeningOffsetX, domeOpeningY, domeOpeningOffsetZ),
                                                     glm::vec3(domeOpeningRadius, 0.06f, domeOpeningRadius));
            myCylinder->draw(lightingShader, domeOpeningMask, roofColor);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glStencilMask(0x00);
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            glm::mat4 m = composeModel(mosqueRoot, glm::vec3(0.0f, kMosqueRoofY, 0.0f), glm::vec3(roofTotalW, 0.5f, roofTotalD));
            myCube->draw(lightingShader, m, roofColor); // Draw main mosque roof slab.

            glStencilMask(0xFF);
            glDisable(GL_STENCIL_TEST);
        }

        // Dome sitting atop the roof
        {
            glBindTexture(GL_TEXTURE_2D, texDome);
            lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
            glm::mat4 m = composeModel(mosqueRoot, glm::vec3(0.0f, kMainDomeY, 0.0f), glm::vec3(domeScale, domeScale, domeScale));
            mySphere->drawDomeHemisphere(lightingShader, m, domeColor); // Draw main mosque dome shell.
        }

        // Main dome cone ornament (made more pointy)
        {
            glBindTexture(GL_TEXTURE_2D, texDome);
            lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
            glm::mat4 m = composeModel(mosqueRoot, glm::vec3(0.0f, kMainDomeOrnamentY, 0.0f), glm::vec3(0.4f, 3.5f, 0.4f));
            myCone->draw(lightingShader, m, goldColor); // Draw main dome top cone ornament.
        }
    }

    // Ceiling fans inside main mosque (6 total, 3 x 2)
    {
        glBindTexture(GL_TEXTURE_2D, texDome);
        lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

        struct FanPos
        {
            float x, z;
        };
        std::vector<FanPos> fans = {
            {-48.0f, -24.0f}, {0.0f, -24.0f}, {48.0f, -24.0f}, {-48.0f, 24.0f}, {0.0f, 24.0f}, {48.0f, 24.0f}};

        for (const auto &f : fans)
        {
            float fx = f.x;
            float fz = f.z;

            // Downrod (stop above hub; do not pass through it)
            float rodYTop = kMosqueRoofY + kFanRodTopYOffsetFromRoof;
            float rodYBottom = kMosqueRoofY + kFanRodBottomYOffsetFromRoof;
            float rodLen = rodYTop - rodYBottom;
            // Anchor the rod at the bottom so scaling only extends upward toward the hub
            glm::mat4 mRod = mosqueRoot;
            mRod = glm::translate(mRod, glm::vec3(fx, rodYBottom, fz));
            mRod = glm::scale(mRod, glm::vec3(0.08f, rodLen * 0.7f, 0.08f));
            mRod = glm::translate(mRod, glm::vec3(0.0f, 1.0f, 0.0f));
            myCylinder->draw(lightingShader, mRod, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw one ceiling-fan downrod.

            // Hub (made larger)
            glm::mat4 mHub = composeModel(mosqueRoot, glm::vec3(fx, kMosqueRoofY + kFanHubYOffsetFromRoof, fz), glm::vec3(0.65f, 0.22f, 0.65f));
            myCylinder->draw(lightingShader, mHub, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw one ceiling-fan hub.

            // 3 blades (made bigger)
            for (int b = 0; b < 3; ++b)
            {
                float a = fanAngleDeg + b * 120.0f;
                glm::mat4 blade = glm::translate(mosqueRoot, glm::vec3(fx, kMosqueRoofY + kFanBladeYOffsetFromRoof, fz));
                blade = glm::rotate(blade, glm::radians(a), glm::vec3(0.0f, 1.0f, 0.0f));
                blade = glm::translate(blade, glm::vec3(2.0f, 0.0f, 0.0f));
                blade = glm::scale(blade, glm::vec3(3.6f, 0.06f, 0.5f));
                myCube->draw(lightingShader, blade, glm::vec3(1.0f, 1.0f, 1.0f)); // Draw one rotating ceiling-fan blade.
            }
        }
    }

    // Veranda roofs are drawn inside their own build functions.
}

// This function draws the intricate curved structure making up the Mihrab
// (the prayer niche pointing to Mecca). It draws its curved walls, textured
// roof cap, and the inner floor platform.
static void drawMihrabWallsAndRoof(
    Shader &lightingShader,
    BezierMihrab *myMihrab,
    GLuint texMihrab,
    GLuint texWall,
    const glm::mat4 &mosqueRoot,
    const glm::vec3 &wallColor,
    const glm::vec3 &roofColor)
{
    // 2c. MIHRAB Curved Wall
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

    {
        // Mihrab Curved Wall
        glBindTexture(GL_TEXTURE_2D, texMihrab);
        myMihrab->drawWall(lightingShader, mosqueRoot, wallColor);
    }
    {
        // Mihrab Curved Roof Cap
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 10.0f, 3.0f);
        myMihrab->drawRoof(lightingShader, mosqueRoot, roofColor);
    }
    {
        // Mihrab Curved Floor Cap (inner platform surface)
        glBindTexture(GL_TEXTURE_2D, texFloor);
        lightingShader.setVec2("uvTiling", 1.0f, -1.0f);
        myMihrab->drawFloor(lightingShader, mosqueRoot, glm::vec3(1.0f));
    }
}

enum class VerandaStage
{
    Base,
    Pillars,
    Roof,
    Lamps
};

enum class MosqueStage
{
    Foundations,
    WallsAndRoof,
    LightVisuals
};

enum class MihrabStage
{
    Base,
    WallsRoofFan
};

// This function constructs and draws the front veranda of the mosque in
// distinct rendering stages: base platform, pillars, roof slab, and hanging lamps.
static void drawFrontVeranda(
    VerandaStage stage,
    Shader &lightingShader,
    Shader &lightCubeShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    GLuint texFloor,
    GLuint texWall,
    const glm::mat4 &mosqueRoot,
    float mosqueZ,
    const glm::mat4 &projection,
    const glm::mat4 &view,
    const glm::vec3 &pillarColor,
    const glm::vec3 &roofColor)
{
    switch (stage)
    {
    case VerandaStage::Base:
    {
        glBindTexture(GL_TEXTURE_2D, texFloor);
        lightingShader.setVec2("uvTiling", 12.0f, 3.0f);
        glm::mat4 m = composeModel(mosqueRoot, glm::vec3(15.0f, 1.5f, 74.5f), glm::vec3(190.0f, 3.0f, 51.0f));
        myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw front veranda base platform.
        break;
    }
    case VerandaStage::Pillars:
    {
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 1.0f, 4.0f);

        float pillarBottomY = 3.0f;
        float pillarTopY = kMosqueRoofY;
        float pillarYCenter = (pillarBottomY + pillarTopY) * 0.5f;
        float pillarHeight = (pillarTopY - pillarBottomY) * 0.5f;

        std::vector<float> frontPillarXs = {-30.0f, 5.0f, 40.0f, 75.0f, 110.0f, 145.0f};
        std::vector<float> frontPillarZs = {mosqueZ + 25.0f, mosqueZ + 45.0f, mosqueZ + 65.0f};
        for (float px : frontPillarXs)
        {
            for (float pz : frontPillarZs)
            {
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(px - 40.0f, pillarYCenter, pz - (mosqueZ - 30.0f)), glm::vec3(1.2f, pillarHeight, 1.2f));
                myCylinder->draw(lightingShader, m, pillarColor); // Draw one front veranda pillar column.
            }
        }
        break;
    }
    case VerandaStage::Roof:
    {
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 10.0f, 3.0f);
        glm::mat4 m = composeModel(mosqueRoot, glm::vec3(15.0f, kMosqueRoofY, 75.0f), glm::vec3(190.0f, 0.5f, 50.0f));
        myCube->draw(lightingShader, m, roofColor); // Draw front veranda roof slab.
        break;
    }
    case VerandaStage::Lamps:
    {
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        glm::vec3 verandaLampColor = pointLightsOn ? glm::vec3(1.0f, 0.92f, 0.75f) : glm::vec3(0.25f, 0.25f, 0.25f);

        // Front veranda: centers of pillar groups (center of each 4-pillar cell)
        std::vector<float> frontPillarXs = {-30.0f, 5.0f, 40.0f, 75.0f, 110.0f, 145.0f};
        std::vector<float> frontPillarZs = {mosqueZ + 25.0f, mosqueZ + 45.0f, mosqueZ + 65.0f};

        for (size_t ix = 0; ix + 1 < frontPillarXs.size(); ++ix)
        {
            for (size_t iz = 0; iz + 1 < frontPillarZs.size(); ++iz)
            {
                float cx = (frontPillarXs[ix] + frontPillarXs[ix + 1]) * 0.5f - 40.0f;
                float cz = (frontPillarZs[iz] + frontPillarZs[iz + 1]) * 0.5f - (mosqueZ - 30.0f);

                glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(cx, kVerandaLampY, cz));
                m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                m = glm::scale(m, glm::vec3(1.55f, 1.55f, 1.55f));

                lightCubeShader.setMat4("model", m);
                lightCubeShader.setVec3("lightColor", verandaLampColor);
                mySphere->drawHemisphere(lightCubeShader, m, verandaLampColor); // Draw one front veranda hanging lamp shade.
            }
        }
        break;
    }
    }
}

// This function constructs and draws the right-side veranda of the mosque in
// distinct rendering stages: base platform, pillars, roof slab, and hanging lamps.
static void drawRightVeranda(
    VerandaStage stage,
    Shader &lightingShader,
    Shader &lightCubeShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    GLuint texFloor,
    GLuint texWall,
    const glm::mat4 &mosqueRoot,
    float mosqueZ,
    const glm::mat4 &projection,
    const glm::mat4 &view,
    const glm::vec3 &pillarColor,
    const glm::vec3 &roofColor)
{
    switch (stage)
    {
    case VerandaStage::Base:
    {
        glBindTexture(GL_TEXTURE_2D, texFloor);
        lightingShader.setVec2("uvTiling", 2.0f, 8.0f);
        glm::mat4 m = composeModel(mosqueRoot, glm::vec3(94.5f, 1.5f, 0.0f), glm::vec3(31.0f, 3.0f, 98.0f));
        myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw right veranda base platform.
        break;
    }
    case VerandaStage::Pillars:
    {
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 1.0f, 4.0f);

        float pillarBottomY = 3.0f;
        float pillarTopY = kMosqueRoofY;
        float pillarYCenter = (pillarBottomY + pillarTopY) * 0.5f;
        float pillarHeight = (pillarTopY - pillarBottomY) * 0.5f;

        float rightPillarX = 145.0f;
        std::vector<float> rightPillarZs = {mosqueZ - 75.0f, mosqueZ - 45.0f, mosqueZ - 15.0f};

        for (float pz : rightPillarZs)
        {
            glm::mat4 m = composeModel(mosqueRoot,
                                       glm::vec3(rightPillarX - 40.0f, pillarYCenter, pz - (mosqueZ - 30.0f)),
                                       glm::vec3(1.2f, pillarHeight, 1.2f));
            myCylinder->draw(lightingShader, m, pillarColor); // Draw one right veranda pillar column.
        }
        break;
    }
    case VerandaStage::Roof:
    {
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 8.0f, 4.0f);
        glm::mat4 m = composeModel(mosqueRoot, glm::vec3(95.0f, kMosqueRoofY, 0.0f), glm::vec3(30.0f, 0.5f, 98.0f));
        myCube->draw(lightingShader, m, roofColor); // Draw right veranda roof slab.
        break;
    }
    case VerandaStage::Lamps:
    {
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        glm::vec3 verandaLampColor = pointLightsOn ? glm::vec3(1.0f, 0.92f, 0.75f) : glm::vec3(0.25f, 0.25f, 0.25f);

        float rightLampX = 95.0f;
        std::vector<float> rightPillarZs = {mosqueZ - 75.0f, mosqueZ - 45.0f, mosqueZ - 15.0f, mosqueZ + 19.0f};

        for (size_t i = 0; i + 1 < rightPillarZs.size(); ++i)
        {
            float cz = (rightPillarZs[i] + rightPillarZs[i + 1]) * 0.5f - (mosqueZ - 30.0f);
            glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(rightLampX, kVerandaLampY, cz));
            m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::scale(m, glm::vec3(1.55f, 1.55f, 1.55f));

            lightCubeShader.setMat4("model", m);
            lightCubeShader.setVec3("lightColor", verandaLampColor);
            mySphere->drawHemisphere(lightCubeShader, m, verandaLampColor); // Draw one right veranda hanging lamp shade.
        }
        break;
    }
    }
}

// This function constructs and draws the complete Mihrab structure protruding
// from the back wall of the mosque in distinct rendering stages: its foundation
// base volume, its walls, roof, and the hanging ceiling fan inside it.
static void drawMihrab(
    MihrabStage stage,
    Shader &lightingShader,
    Cube *myCube,
    Cylinder *myCylinder,
    GLuint texFloor,
    GLuint texMihrab,
    GLuint texWall,
    GLuint texDome,
    const glm::mat4 &mosqueRoot,
    const glm::vec3 &wallColor,
    const glm::vec3 &roofColor)
{
    switch (stage)
    {
    case MihrabStage::Base:
    {
        glBindTexture(GL_TEXTURE_2D, texFloor);
        lightingShader.setVec2("uvTiling", 1.0f, -1.0f);
        // Draw the curved base volume extending behind the mosque
        myBezierMihrab->drawBaseVolume(lightingShader, mosqueRoot, glm::vec3(1.0f));
        break;
    }
    case MihrabStage::WallsRoofFan:
    {
        // Match the uvTiling state the old call site had.
        lightingShader.setVec2("uvTiling", 10.0f, 3.0f);
        drawMihrabWallsAndRoof(lightingShader, myBezierMihrab, texMihrab, texWall, mosqueRoot, wallColor, roofColor);

        // Mihrab fan (single fan replacing the one removed from the main-fans list).
        glBindTexture(GL_TEXTURE_2D, texDome);
        lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

        float fx = 0.0f;
        float fz = -58.5f;

        float rodTopY = kMosqueRoofY + kFanRodTopYOffsetFromRoof;
        float rodBottomY = kMosqueRoofY + kFanRodBottomYOffsetFromRoof;
        float rodLen = rodTopY - rodBottomY;

        // Anchor mihrab fan rod at bottom and scale upward
        glm::mat4 mRod = mosqueRoot;
        mRod = glm::translate(mRod, glm::vec3(fx, rodBottomY, fz));
        mRod = glm::scale(mRod, glm::vec3(0.08f, rodLen * 0.7f, 0.08f));
        mRod = glm::translate(mRod, glm::vec3(0.0f, 1.0f, 0.0f));
        myCylinder->draw(lightingShader, mRod, glm::vec3(1.0f)); // Draw mihrab ceiling-fan downrod.

        glm::mat4 mHub = composeModel(mosqueRoot, glm::vec3(fx, kMosqueRoofY + kFanHubYOffsetFromRoof, fz), glm::vec3(0.65f, 0.22f, 0.65f));
        myCylinder->draw(lightingShader, mHub, glm::vec3(1.0f)); // Draw mihrab ceiling-fan hub.

        for (int b = 0; b < 3; ++b)
        {
            float a = fanAngleDeg + b * 120.0f;
            glm::mat4 blade = glm::translate(mosqueRoot, glm::vec3(fx, kMosqueRoofY + kFanBladeYOffsetFromRoof, fz));
            blade = glm::rotate(blade, glm::radians(a), glm::vec3(0.0f, 1.0f, 0.0f));
            blade = glm::translate(blade, glm::vec3(2.0f, 0.0f, 0.0f));
            blade = glm::scale(blade, glm::vec3(3.6f, 0.06f, 0.5f));
            myCube->draw(lightingShader, blade, glm::vec3(1.0f)); // Draw one mihrab ceiling-fan blade.
        }

        break;
    }
    }
}

// This function draws the wooden Minbar (pulpit / chair with stairs) located
// inside the mosque near the right side of the Mihrab. It draws the stairs,
// the seating area, back panel, and decorative railings.
static void drawMihrabChairWithStairs(
    Shader &lightingShader,
    Cube *myCube,
    const glm::mat4 &mosqueRoot)
{
    constexpr float kChairBaseY = 3.0f;
    constexpr float kChairCenterX = 9.8f;
    constexpr float kStairFrontZ = -54.4f;
    constexpr float kStepHeight = 1.15f;
    constexpr float kStepRun = 1.65f;

    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setFloat("materialTexBlend", 0.4f);
    lightingShader.setVec2("uvTiling", 1.6f, 1.6f);

    const glm::vec3 polishedTint = glm::vec3(1.45f, 1.25f, 0.55f);
    const glm::vec3 trimTint = glm::vec3(1.30f, 1.10f, 0.45f);
    const glm::vec3 darkTrim = glm::vec3(0.95f, 0.80f, 0.35f);

    // Stair flight: 4 steps ascending toward the back.
    for (int i = 0; i < 4; ++i)
    {
        float topY = kChairBaseY + (i + 1) * kStepHeight;
        float blockH = topY - kChairBaseY;
        float stepCenterY = kChairBaseY + blockH * 0.5f;
        float stepCenterZ = kStairFrontZ - i * kStepRun;
        glm::mat4 stepModel = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX, stepCenterY, stepCenterZ),
            glm::vec3(4.8f, blockH, 1.5f));
        myCube->draw(lightingShader, stepModel, polishedTint);

        // Step nosing trim (thin decorative strip on the front edge of each step)
        {
            glm::mat4 nosing = composeModel(mosqueRoot,
                                            glm::vec3(kChairCenterX, topY - 0.08f, stepCenterZ + 0.75f + 0.08f),
                                            glm::vec3(4.9f, 0.15f, 0.15f));
            myCube->draw(lightingShader, nosing, darkTrim);
        }
    }

    // Landing and main seat body.
    {
        float landingTopY = kChairBaseY + 4.0f * kStepHeight;
        float landingH = landingTopY - kChairBaseY;
        glm::mat4 landing = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX, kChairBaseY + landingH * 0.5f, -60.7f),
            glm::vec3(5.8f, landingH, 2.9f));
        myCube->draw(lightingShader, landing, polishedTint);

        glm::mat4 seat = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX, landingTopY + 0.75f, -60.8f),
            glm::vec3(4.6f, 1.5f, 3.5f));
        myCube->draw(lightingShader, seat, polishedTint);
    }

    // Back panel and side cheeks.
    {
        glm::mat4 backPanel = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX, 11.8f, -62.7f),
            glm::vec3(4.6f, 6.2f, 0.8f));
        myCube->draw(lightingShader, backPanel, trimTint);

        glm::mat4 leftCheek = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX - 2.65f, 8.5f, -58.0f),
            glm::vec3(0.55f, 4.8f, 7.0f));
        myCube->draw(lightingShader, leftCheek, trimTint);

        glm::mat4 rightCheek = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX + 2.65f, 8.5f, -58.0f),
            glm::vec3(0.55f, 4.8f, 7.0f));
        myCube->draw(lightingShader, rightCheek, trimTint);

        glm::mat4 topCap = composeModel(
            mosqueRoot,
            glm::vec3(kChairCenterX, 15.2f, -62.65f),
            glm::vec3(5.1f, 0.45f, 1.05f));
        myCube->draw(lightingShader, topCap, trimTint);
    }

    // --- Side railing panels along the staircase ---
    {
        // Continuous panels running alongside the stairs from front to landing
        float railFrontZ = kStairFrontZ + 0.6f;
        float railBackZ = -59.5f;
        float railLen = railFrontZ - railBackZ;
        float railMidZ = (railFrontZ + railBackZ) * 0.5f;
        float railH = 3.2f;
        float railBottomY = kChairBaseY + 1.5f;

        // Left railing
        glm::mat4 mL = composeModel(mosqueRoot,
                                    glm::vec3(kChairCenterX - 2.65f, railBottomY + railH * 0.5f, railMidZ),
                                    glm::vec3(0.25f, railH, railLen));
        myCube->draw(lightingShader, mL, darkTrim);

        // Right railing
        glm::mat4 mR = composeModel(mosqueRoot,
                                    glm::vec3(kChairCenterX + 2.65f, railBottomY + railH * 0.5f, railMidZ),
                                    glm::vec3(0.25f, railH, railLen));
        myCube->draw(lightingShader, mR, darkTrim);

        // Top rail bars (horizontal strips atop the railings)
        float topRailY = railBottomY + railH + 0.15f;
        glm::mat4 mLT = composeModel(mosqueRoot,
                                     glm::vec3(kChairCenterX - 2.65f, topRailY, railMidZ),
                                     glm::vec3(0.4f, 0.25f, railLen + 0.3f));
        myCube->draw(lightingShader, mLT, trimTint);

        glm::mat4 mRT = composeModel(mosqueRoot,
                                     glm::vec3(kChairCenterX + 2.65f, topRailY, railMidZ),
                                     glm::vec3(0.4f, 0.25f, railLen + 0.3f));
        myCube->draw(lightingShader, mRT, trimTint);
    }

    lightingShader.setFloat("materialTexBlend", 0.0f);
}

// ---------------------------------------------------------------------------
// Draw an alcove-style bookshelf recessed into the back wall.
// The alcove sits in the window opening of a back-wall segment.
//   alcoveCenterX : center X in mosque-local coordinates
//   alcoveWidth   : full width of the opening (x)
//   alcoveHeight  : full height of the opening (y)
//   alcoveBottomY : Y coordinate of the bottom of the opening
//   wallZ         : Z coordinate of the wall plane
//   alcoveDepth   : how far the alcove recesses behind the wall (into -Z)
// ---------------------------------------------------------------------------
static void drawBackWallAlcoveBookshelf(
    Shader &lightingShader,
    Cube *myCube,
    GLuint texWood,
    GLuint texWall,
    GLuint texBook,
    const glm::mat4 &mosqueRoot,
    float alcoveCenterX,
    float alcoveWidth,
    float alcoveHeight,
    float alcoveBottomY,
    float wallZ,
    float alcoveDepth,
    const glm::vec3 &wallColor)
{
    // Wood color for shelves / alcove frame
    const glm::vec3 woodColor(0.55f, 0.35f, 0.18f);
    const glm::vec3 darkWood(0.40f, 0.24f, 0.12f);
    const glm::vec3 backPanelColor(0.82f, 0.76f, 0.65f);

    const float panelThickness = 0.4f;  // thickness of side/top/bottom panels
    const float shelfThickness = 0.35f; // thickness of each horizontal shelf

    const float alcoveLeft = alcoveCenterX - alcoveWidth * 0.5f;
    const float alcoveRight = alcoveCenterX + alcoveWidth * 0.5f;
    const float alcoveTopY = alcoveBottomY + alcoveHeight;

    // The alcove recesses behind the wall plane (into -Z direction)
    const float backZ = wallZ - alcoveDepth;
    const float midZ = wallZ - alcoveDepth * 0.5f;

    // --- Back panel (flush with the back of the alcove) ---
    glBindTexture(GL_TEXTURE_2D, texWall);
    lightingShader.setVec2("uvTiling", 2.0f, 1.0f);
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveBottomY + alcoveHeight * 0.5f, backZ),
                                   glm::vec3(alcoveWidth, alcoveHeight, panelThickness * 0.5f));
        myCube->draw(lightingShader, m, backPanelColor); // Alcove back panel.
    }

    // --- Top panel ---
    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveTopY - panelThickness * 0.5f, midZ),
                                   glm::vec3(alcoveWidth, panelThickness, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove top panel.
    }

    // --- Bottom panel ---
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveBottomY + panelThickness * 0.5f, midZ),
                                   glm::vec3(alcoveWidth, panelThickness, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove bottom panel.
    }

    // --- Left side panel ---
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveLeft + panelThickness * 0.5f, alcoveBottomY + alcoveHeight * 0.5f, midZ),
                                   glm::vec3(panelThickness, alcoveHeight, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove left side panel.
    }

    // --- Right side panel ---
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveRight - panelThickness * 0.5f, alcoveBottomY + alcoveHeight * 0.5f, midZ),
                                   glm::vec3(panelThickness, alcoveHeight, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove right side panel.
    }

    // --- Horizontal shelves ---
    // Divide the alcove interior into shelf compartments.
    // Interior height = alcoveHeight - 2 * panelThickness
    const float interiorBottom = alcoveBottomY + panelThickness;
    const float interiorTop = alcoveTopY - panelThickness;
    const float interiorHeight = interiorTop - interiorBottom;
    const float interiorLeft = alcoveLeft + panelThickness;
    const float interiorRight = alcoveRight - panelThickness;
    const float interiorWidth = interiorRight - interiorLeft;

    const int numShelves = 3; // 3 horizontal shelves create 4 compartments
    const float compartmentHeight = interiorHeight / (numShelves + 1);

    for (int i = 1; i <= numShelves; ++i)
    {
        float shelfY = interiorBottom + compartmentHeight * i;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, shelfY, midZ),
                                   glm::vec3(interiorWidth, shelfThickness, alcoveDepth - panelThickness * 0.5f));
        myCube->draw(lightingShader, m, woodColor); // Alcove horizontal shelf.
    }

    // --- Center vertical divider for visual interest ---
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, interiorBottom + interiorHeight * 0.5f, midZ),
                                   glm::vec3(panelThickness * 0.6f, interiorHeight, alcoveDepth - panelThickness * 0.5f));
        myCube->draw(lightingShader, m, darkWood); // Alcove center divider.
    }

    // --- Books on shelves ---
    // Book colors (leather-bound look)
    const glm::vec3 bookColors[] = {
        glm::vec3(0.60f, 0.15f, 0.12f), // dark red
        glm::vec3(0.14f, 0.35f, 0.18f), // dark green
        glm::vec3(0.18f, 0.22f, 0.52f), // navy blue
        glm::vec3(0.50f, 0.38f, 0.15f), // brown/tan
        glm::vec3(0.70f, 0.55f, 0.20f), // gold/amber
        glm::vec3(0.35f, 0.12f, 0.30f), // plum
        glm::vec3(0.22f, 0.42f, 0.45f), // teal
        glm::vec3(0.55f, 0.25f, 0.10f), // chestnut
    };
    const int numBookColors = 8;

    // Bind book texture and blend with book colors for a tinted leather look
    glBindTexture(GL_TEXTURE_2D, texBook);
    lightingShader.setFloat("materialTexBlend", 0.45f);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

    // For each compartment (4 total = numShelves + 1), place a couple of books
    // Books sit on the shelf surface. In the bottom compartment, they sit on the bottom panel.
    for (int comp = 0; comp <= numShelves; ++comp)
    {
        float shelfSurfaceY;
        if (comp == 0)
            shelfSurfaceY = interiorBottom; // sits on bottom panel
        else
            shelfSurfaceY = interiorBottom + compartmentHeight * comp + shelfThickness * 0.5f;

        float availableH = compartmentHeight - shelfThickness;
        if (comp == 0)
            availableH = compartmentHeight; // first compartment has no shelf below

        // Place books in left half and right half (divided by center divider)
        float halfSections[2][2] = {
            {interiorLeft + 0.3f, alcoveCenterX - panelThickness * 0.4f},
            {alcoveCenterX + panelThickness * 0.4f, interiorRight - 0.3f}};

        for (int half = 0; half < 2; ++half)
        {
            float secLeft = halfSections[half][0];
            float secRight = halfSections[half][1];

            // --- Helper lambda-style pseudo-random from seed ---
            // We'll use a mutable seed that we advance per book.
            unsigned int rngState = (unsigned int)(comp * 137 + half * 59 + (int)(alcoveCenterX * 7.0f) + 12345u);
            auto nextRng = [&rngState]() -> float
            {
                rngState = rngState * 1103515245u + 12345u;
                return (float)((rngState >> 16) & 0x7FFF) / 32767.0f; // 0..1
            };

            // --- Determine random book count for outer cluster (2-4) ---
            int outerCount = 2 + (int)(nextRng() * 2.99f); // 2, 3, or 4
            if (outerCount > 4)
                outerCount = 4;

            // --- Determine random book count near center divider (1-2) ---
            int innerCount = 1 + (int)(nextRng() * 1.99f); // 1 or 2
            if (innerCount > 2)
                innerCount = 2;

            // --- Draw outer cluster (against the side wall) ---
            {
                float cursorX;
                float direction; // +1 means placing left-to-right, -1 right-to-left
                if (half == 0)
                {
                    // Left half: books lean against the left wall
                    cursorX = secLeft + 0.25f;
                    direction = 1.0f;
                }
                else
                {
                    // Right half: books lean against the right wall
                    cursorX = secRight - 0.25f;
                    direction = -1.0f;
                }

                for (int b = 0; b < outerCount; ++b)
                {
                    float bw = 0.85f + nextRng() * 0.55f;         // width 0.85 - 1.4
                    float heightFrac = 0.72f + nextRng() * 0.22f; // 0.72 - 0.94
                    float bookH = availableH * heightFrac;
                    if (bookH < 1.3f)
                        bookH = 1.3f;
                    if (bookH > availableH - 0.25f)
                        bookH = availableH - 0.25f;

                    float bookDepth = alcoveDepth * 0.50f + alcoveDepth * 0.25f * nextRng();
                    float gap = 0.15f + nextRng() * 0.25f; // small random gap 0.15-0.4

                    float bx = cursorX + direction * (bw * 0.5f);
                    float by = shelfSurfaceY + bookH * 0.5f;
                    float bz = wallZ - alcoveDepth * 0.5f + (alcoveDepth * 0.12f) * (nextRng() - 0.5f);

                    int colorIdx = (comp * 5 + half * 3 + b * 2 + outerCount) % numBookColors;

                    glm::mat4 m = composeModel(mosqueRoot,
                                               glm::vec3(bx, by, bz),
                                               glm::vec3(bw, bookH, bookDepth));
                    myCube->draw(lightingShader, m, bookColors[colorIdx]); // Outer book.

                    cursorX += direction * (bw + gap);
                }
            }

            // --- Draw inner cluster (leaning against the center divider) ---
            {
                float cursorX;
                float direction;
                if (half == 0)
                {
                    // Left half: inner books lean against center divider (right side)
                    cursorX = alcoveCenterX - panelThickness * 0.4f - 0.2f;
                    direction = -1.0f;
                }
                else
                {
                    // Right half: inner books lean against center divider (left side)
                    cursorX = alcoveCenterX + panelThickness * 0.4f + 0.2f;
                    direction = 1.0f;
                }

                for (int b = 0; b < innerCount; ++b)
                {
                    float bw = 0.80f + nextRng() * 0.50f; // width 0.80 - 1.3
                    float heightFrac = 0.70f + nextRng() * 0.24f;
                    float bookH = availableH * heightFrac;
                    if (bookH < 1.3f)
                        bookH = 1.3f;
                    if (bookH > availableH - 0.25f)
                        bookH = availableH - 0.25f;

                    float bookDepth = alcoveDepth * 0.48f + alcoveDepth * 0.22f * nextRng();
                    float gap = 0.12f + nextRng() * 0.20f;

                    float bx = cursorX + direction * (bw * 0.5f);
                    float by = shelfSurfaceY + bookH * 0.5f;
                    float bz = wallZ - alcoveDepth * 0.5f + (alcoveDepth * 0.10f) * (nextRng() - 0.5f);

                    int colorIdx = (comp * 7 + half * 11 + b * 3 + innerCount) % numBookColors;

                    glm::mat4 m = composeModel(mosqueRoot,
                                               glm::vec3(bx, by, bz),
                                               glm::vec3(bw, bookH, bookDepth));
                    myCube->draw(lightingShader, m, bookColors[colorIdx]); // Inner book.

                    cursorX += direction * (bw + gap);
                }
            }
        }
    }

    // Restore texture blend
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // --- Decorative arch trim at the top of the alcove (pointed arch silhouette) ---
    // Trims only extend inward (from wallZ toward backZ), NOT past wallZ
    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    {
        // Header trim: centered between wallZ and backZ, no overhang past wallZ
        float trimMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveTopY + 0.15f, trimMidZ),
                                   glm::vec3(alcoveWidth + 1.2f, 0.6f, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove decorative header trim.
    }
    // Bottom decorative trim / baseboard
    {
        float trimMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveBottomY - 0.15f, trimMidZ),
                                   glm::vec3(alcoveWidth + 1.2f, 0.5f, alcoveDepth));
        myCube->draw(lightingShader, m, darkWood); // Alcove decorative baseboard trim.
    }

    // --- Exterior cover panels ---
    // Wall-textured panels on the outside of the back wall to hide
    // the alcove wood structure from the exterior.
    glBindTexture(GL_TEXTURE_2D, texWall);
    lightingShader.setVec2("uvTiling", 2.0f, 1.0f);

    // Back cover: flush behind the alcove back panel
    {
        float coverZ = backZ - 0.05f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveBottomY + alcoveHeight * 0.5f, coverZ),
                                   glm::vec3(alcoveWidth + 1.5f, alcoveHeight + 1.5f, 0.15f));
        myCube->draw(lightingShader, m, wallColor); // Exterior back cover.
    }

    // Left side cover: hides left side panel from outside
    {
        float coverMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveLeft - 0.15f, alcoveBottomY + alcoveHeight * 0.5f, coverMidZ),
                                   glm::vec3(0.8f, alcoveHeight + 1.5f, alcoveDepth + 0.5f));
        myCube->draw(lightingShader, m, wallColor); // Exterior left side cover.
    }

    // Right side cover: hides right side panel from outside
    {
        float coverMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveRight + 0.15f, alcoveBottomY + alcoveHeight * 0.5f, coverMidZ),
                                   glm::vec3(0.8f, alcoveHeight + 1.5f, alcoveDepth + 0.5f));
        myCube->draw(lightingShader, m, wallColor); // Exterior right side cover.
    }

    // Top cover: hides top panel and header trim from outside
    {
        float coverMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveTopY + 0.45f, coverMidZ),
                                   glm::vec3(alcoveWidth + 1.5f, 1.5f, alcoveDepth + 0.5f));
        myCube->draw(lightingShader, m, wallColor); // Exterior top cover.
    }

    // Bottom cover: hides bottom panel and baseboard trim from outside
    {
        float coverMidZ = wallZ - alcoveDepth * 0.5f;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(alcoveCenterX, alcoveBottomY - 0.45f, coverMidZ),
                                   glm::vec3(alcoveWidth + 1.5f, 1.5f, alcoveDepth + 0.5f));
        myCube->draw(lightingShader, m, wallColor); // Exterior bottom cover.
    }

    lightingShader.setFloat("materialTexBlend", 0.0f);
}
// ---------------------------------------------------------------------------
// Draw a low-height bookshelf for the back-wall corners, with books inside
// and animated front door panels toggled by the B key.
// ---------------------------------------------------------------------------
static void drawCornerBookshelf(
    Shader &lightingShader,
    Cube *myCube,
    GLuint texWood,
    GLuint texBook,
    const glm::mat4 &mosqueRoot,
    float centerX,
    float bottomY,
    float backZ,
    float width,
    float height,
    float depth)
{
    const glm::vec3 woodColor(0.55f, 0.35f, 0.18f);
    const glm::vec3 darkWood(0.40f, 0.24f, 0.12f);
    const glm::vec3 lightWood(0.72f, 0.55f, 0.32f);
    const float panelT = 0.35f;
    const float shelfT = 0.28f;

    float topY = bottomY + height;
    float frontZ = backZ + depth;
    float midZ = backZ + depth * 0.5f;
    float halfW = width * 0.5f;
    float left = centerX - halfW;
    float right = centerX + halfW;

    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setFloat("materialTexBlend", 0.40f);
    lightingShader.setVec2("uvTiling", 1.5f, 1.5f);

    // Back panel
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, bottomY + height * 0.5f, backZ + panelT * 0.25f),
                                   glm::vec3(width, height, panelT * 0.5f));
        myCube->draw(lightingShader, m, lightWood);
    }

    // Top panel
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, topY - panelT * 0.5f, midZ),
                                   glm::vec3(width + 0.2f, panelT, depth + 0.2f));
        myCube->draw(lightingShader, m, darkWood);
    }

    // Bottom panel
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, bottomY + panelT * 0.5f, midZ),
                                   glm::vec3(width, panelT, depth));
        myCube->draw(lightingShader, m, darkWood);
    }

    // Left side panel
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(left + panelT * 0.5f, bottomY + height * 0.5f, midZ),
                                   glm::vec3(panelT, height, depth));
        myCube->draw(lightingShader, m, darkWood);
    }

    // Right side panel
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(right - panelT * 0.5f, bottomY + height * 0.5f, midZ),
                                   glm::vec3(panelT, height, depth));
        myCube->draw(lightingShader, m, darkWood);
    }

    // Interior dimensions
    float intBottom = bottomY + panelT;
    float intTop = topY - panelT;
    float intH = intTop - intBottom;
    float intLeft = left + panelT;
    float intRight = right - panelT;
    float intWidth = intRight - intLeft;

    // 2 horizontal shelves -> 3 compartments
    const int numShelves = 2;
    float compH = intH / (numShelves + 1);

    for (int i = 1; i <= numShelves; ++i)
    {
        float shelfY = intBottom + compH * i;
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, shelfY, midZ),
                                   glm::vec3(intWidth, shelfT, depth - panelT));
        myCube->draw(lightingShader, m, woodColor);
    }

    // Center vertical divider
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, intBottom + intH * 0.5f, midZ),
                                   glm::vec3(panelT * 0.5f, intH, depth - panelT));
        myCube->draw(lightingShader, m, darkWood);
    }

    // --- Books on shelves ---
    const glm::vec3 bookColors[] = {
        glm::vec3(0.60f, 0.15f, 0.12f),
        glm::vec3(0.14f, 0.35f, 0.18f),
        glm::vec3(0.18f, 0.22f, 0.52f),
        glm::vec3(0.50f, 0.38f, 0.15f),
        glm::vec3(0.70f, 0.55f, 0.20f),
        glm::vec3(0.35f, 0.12f, 0.30f),
        glm::vec3(0.22f, 0.42f, 0.45f),
        glm::vec3(0.55f, 0.25f, 0.10f),
    };
    const int numBookColors = 8;

    glBindTexture(GL_TEXTURE_2D, texBook);
    lightingShader.setFloat("materialTexBlend", 0.45f);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

    for (int comp = 0; comp <= numShelves; ++comp)
    {
        float surfaceY;
        if (comp == 0)
            surfaceY = intBottom;
        else
            surfaceY = intBottom + compH * comp + shelfT * 0.5f;

        float availH = compH - shelfT;
        if (comp == 0)
            availH = compH;

        float halfSections[2][2] = {
            {intLeft + 0.2f, centerX - panelT * 0.3f},
            {centerX + panelT * 0.3f, intRight - 0.2f}};

        for (int half = 0; half < 2; ++half)
        {
            unsigned int rng = (unsigned int)(comp * 137 + half * 59 + (int)(centerX * 7.0f) + 54321u);
            auto nextR = [&rng]() -> float
            {
                rng = rng * 1103515245u + 12345u;
                return (float)((rng >> 16) & 0x7FFF) / 32767.0f;
            };

            int bookCount = 2 + (int)(nextR() * 2.99f);
            if (bookCount > 4)
                bookCount = 4;

            float cursorX = (half == 0) ? halfSections[half][0] + 0.2f : halfSections[half][1] - 0.2f;
            float dir = (half == 0) ? 1.0f : -1.0f;

            for (int b = 0; b < bookCount; ++b)
            {
                float bw = 0.55f + nextR() * 0.35f;
                float hFrac = 0.70f + nextR() * 0.25f;
                float bookH = availH * hFrac;
                if (bookH < 0.6f)
                    bookH = 0.6f;
                if (bookH > availH - 0.1f)
                    bookH = availH - 0.1f;

                float bookD = depth * 0.42f + depth * 0.2f * nextR();
                float gap = 0.08f + nextR() * 0.12f;
                float bx = cursorX + dir * (bw * 0.5f);
                float by = surfaceY + bookH * 0.5f;
                float bz = midZ + (depth * 0.06f) * (nextR() - 0.5f);

                int ci = (comp * 5 + half * 3 + b * 2 + bookCount) % numBookColors;
                glm::mat4 m = composeModel(mosqueRoot,
                                           glm::vec3(bx, by, bz),
                                           glm::vec3(bw, bookH, bookD));
                myCube->draw(lightingShader, m, bookColors[ci]);

                cursorX += dir * (bw + gap);
            }
        }
    }

    lightingShader.setFloat("materialTexBlend", 0.0f);

    // --- Front door panels (animated with B key) ---
    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setFloat("materialTexBlend", 0.35f);
    lightingShader.setVec2("uvTiling", 1.0f, 2.0f);

    float doorLeafW = (intWidth - 0.4f) * 0.5f;
    float doorLeafH = intH;
    float doorLeafT = 0.2f;
    float doorY = intBottom + doorLeafH * 0.5f;
    float doorZ = frontZ - 0.05f;
    float doorAngle = glm::radians(100.0f * bookshelfOpenAmount);

    // Left door leaf
    {
        float hingeX = intLeft;
        glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(hingeX, doorY, doorZ));
        m = glm::rotate(m, -doorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::translate(m, glm::vec3(doorLeafW * 0.5f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(doorLeafW, doorLeafH, doorLeafT));
        myCube->draw(lightingShader, m, woodColor);
    }

    // Right door leaf
    {
        float hingeX = intRight;
        glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(hingeX, doorY, doorZ));
        m = glm::rotate(m, doorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::translate(m, glm::vec3(-doorLeafW * 0.5f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(doorLeafW, doorLeafH, doorLeafT));
        myCube->draw(lightingShader, m, woodColor);
    }

    // Small door handle knobs (visible when closed)
    if (bookshelfOpenAmount < 0.15f)
    {
        float knobY = doorY;
        float knobZ = doorZ + doorLeafT * 0.5f + 0.06f;
        // Left door knob
        {
            glm::mat4 m = composeModel(mosqueRoot,
                                       glm::vec3(centerX - 0.35f, knobY, knobZ),
                                       glm::vec3(0.12f, 0.12f, 0.08f));
            myCube->draw(lightingShader, m, darkWood);
        }
        // Right door knob
        {
            glm::mat4 m = composeModel(mosqueRoot,
                                       glm::vec3(centerX + 0.35f, knobY, knobZ),
                                       glm::vec3(0.12f, 0.12f, 0.08f));
            myCube->draw(lightingShader, m, darkWood);
        }
    }

    // Decorative top edge trim on front face
    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setVec2("uvTiling", 2.0f, 1.0f);
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, topY + 0.1f, frontZ - 0.05f),
                                   glm::vec3(width + 0.5f, 0.3f, 0.5f));
        myCube->draw(lightingShader, m, darkWood);
    }

    // Base plinth
    {
        glm::mat4 m = composeModel(mosqueRoot,
                                   glm::vec3(centerX, bottomY - 0.1f, midZ),
                                   glm::vec3(width + 0.3f, 0.2f, depth + 0.3f));
        myCube->draw(lightingShader, m, darkWood);
    }

    lightingShader.setFloat("materialTexBlend", 0.0f);
}

// ---------------------------------------------------------------------------
// Draw a Rehal (Quran stand): an X-frame book rest with an open book on top.
// posX/posY/posZ is the base center in mosque-local coordinates.
// ---------------------------------------------------------------------------
static void drawRehal(
    Shader &lightingShader,
    Cube *myCube,
    GLuint texWood,
    GLuint texBook,
    const glm::mat4 &mosqueRoot,
    float posX, float posY, float posZ,
    float scale = 1.0f)
{
    const glm::vec3 rehalWood(0.60f, 0.38f, 0.18f);
    const glm::vec3 rehalDark(0.45f, 0.28f, 0.14f);

    glm::mat4 rehalRoot = glm::translate(mosqueRoot, glm::vec3(posX, posY, posZ));
    rehalRoot = glm::scale(rehalRoot, glm::vec3(scale, scale, scale));
    rehalRoot = glm::rotate(rehalRoot, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glBindTexture(GL_TEXTURE_2D, texWood);
    lightingShader.setFloat("materialTexBlend", 0.40f);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);

    // Base block (flat bottom piece)
    {
        glm::mat4 m = composeModel(rehalRoot,
                                   glm::vec3(0.0f, 0.08f, 0.0f),
                                   glm::vec3(1.8f, 0.16f, 1.0f));
        myCube->draw(lightingShader, m, rehalDark);
    }

    // Left leg (angled board leaning to -Z)
    {
        glm::mat4 m = glm::translate(rehalRoot, glm::vec3(0.0f, 0.55f, -0.25f));
        m = glm::rotate(m, glm::radians(28.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(1.8f, 0.1f, 1.2f));
        myCube->draw(lightingShader, m, rehalWood);
    }

    // Right leg (angled board leaning to +Z)
    {
        glm::mat4 m = glm::translate(rehalRoot, glm::vec3(0.0f, 0.55f, 0.25f));
        m = glm::rotate(m, glm::radians(-28.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(1.8f, 0.1f, 1.2f));
        myCube->draw(lightingShader, m, rehalWood);
    }

    // Small cross brace at the base
    {
        glm::mat4 m = composeModel(rehalRoot,
                                   glm::vec3(0.0f, 0.25f, 0.0f),
                                   glm::vec3(0.15f, 0.35f, 0.8f));
        myCube->draw(lightingShader, m, rehalDark);
    }

    lightingShader.setFloat("materialTexBlend", 0.0f);
}

// This function constructs and draws the core structure of the main mosque building
// in distinct rendering stages: the foundation slab, the main walls (with door/window
// cutouts), the architectural pieces like the alcove bookshelves, corner bookshelves,
// the roof and dome, and lastly the light visuals like the chandelier.
static void drawMainMosque(
    MosqueStage stage,
    Shader &lightingShader,
    Shader &lightCubeShader,
    Cube *myCube,
    Cylinder *myCylinder,
    Sphere *mySphere,
    Cone *myCone,
    GLuint texFloor,
    GLuint texWall,
    GLuint texDome,
    GLuint texMihrab,
    const glm::mat4 &mosqueRoot,
    const glm::mat4 &projection,
    const glm::mat4 &view,
    const glm::vec3 &wallColor,
    const glm::vec3 &domeColor,
    const glm::vec3 &roofColor,
    const glm::vec3 &goldColor,
    const glm::vec3 &spotLightPos,
    const glm::vec3 &spotLightColor)
{
    switch (stage)
    {
    case MosqueStage::Foundations:
    {
        glBindTexture(GL_TEXTURE_2D, texFloor);
        lightingShader.setVec2("uvTiling", 10.0f, 6.0f);
        glm::mat4 m1 = composeModel(mosqueRoot, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(158.0f, 3.0f, 98.0f));
        myCube->draw(lightingShader, m1, glm::vec3(1.0f)); // Draw main mosque foundation/base slab.
        break;
    }
    case MosqueStage::WallsAndRoof:
    {
        // 2 & 3. Side Walls (Left and Right)
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 6.0f, 3.0f);
        float sideWallXs[2] = {-78.5f, 78.5f};
        float doorHeight = 16.0f;
        float doorWidth = 16.0f;
        float totalWallHeight = kMosqueWallHeight;
        float topWallHeight = totalWallHeight - doorHeight;
        float baseWallY = 2.0f;
        float topWallCenterY = baseWallY + doorHeight + (topWallHeight * 0.5f);
        float bottomWallCenterY = baseWallY + (doorHeight * 0.5f);

        float wallZStart = -49.0f;
        float wallZEnd = 49.0f;
        float totalDepth = wallZEnd - wallZStart;
        float centerZ = (wallZStart + wallZEnd) * 0.5f;

        // Local Z coordinates of side veranda pillars
        std::vector<float> sidePillarZs = {-45.0f, -15.0f, 15.0f, 49.0f};
        std::vector<float> doorGapsStart;
        std::vector<float> doorGapsEnd;

        // Calculate door bounds centered exactly midway between the side pillars
        for (size_t i = 0; i < sidePillarZs.size() - 1; ++i)
        {
            float midZ = (sidePillarZs[i] + sidePillarZs[i + 1]) * 0.5f;
            doorGapsStart.push_back(midZ - (doorWidth * 0.5f));
            doorGapsEnd.push_back(midZ + (doorWidth * 0.5f));
        }

        for (float sideX : sideWallXs)
        {
            glm::mat4 mTop = composeModel(mosqueRoot, glm::vec3(sideX, topWallCenterY, centerZ), glm::vec3(1.0f, topWallHeight, totalDepth));
            myCube->draw(lightingShader, mTop, wallColor); // Draw upper side-wall strip above side doors/windows.

            // Bottom solid wall segments (skipping the empty door gaps)
            float currentZ = wallZStart;
            for (size_t i = 0; i < doorGapsStart.size(); ++i)
            {
                float nextDoorStart = doorGapsStart[i];
                if (nextDoorStart > wallZEnd)
                    break;

                if (nextDoorStart > currentZ)
                {
                    float zSpan = nextDoorStart - currentZ;
                    float centerZ = currentZ + (zSpan * 0.5f);
                    glm::mat4 mBot = composeModel(mosqueRoot, glm::vec3(sideX, bottomWallCenterY, centerZ),
                                                  glm::vec3(1.0f, doorHeight, zSpan));
                    myCube->draw(lightingShader, mBot, wallColor); // Draw one lower side-wall segment between openings.
                }

                // For left wall: door gaps become windows by adding a sill
                if (sideX < 0.0f && nextDoorStart <= wallZEnd)
                {
                    float gapStart = nextDoorStart;
                    float gapEnd = std::min(doorGapsEnd[i], wallZEnd);
                    float depth = gapEnd - gapStart;
                    if (depth > 0.0f)
                    {
                        float blockCenterZ = gapStart + (depth * 0.5f);
                        float sillHeight = 6.0f;
                        float sillCenterY = baseWallY + (sillHeight * 0.5f);
                        glm::mat4 mSill = composeModel(mosqueRoot, glm::vec3(sideX, sillCenterY, blockCenterZ), glm::vec3(1.0f, sillHeight, depth));
                        myCube->draw(lightingShader, mSill, wallColor); // Draw left-side window sill/infill block.
                    }
                }

                currentZ = doorGapsEnd[i];
            }

            // Final bottom wall segment finishing the wall length
            if (currentZ < wallZEnd)
            {
                float zSpan = wallZEnd - currentZ;
                float centerZ = currentZ + (zSpan * 0.5f);
                glm::mat4 mBot = composeModel(mosqueRoot, glm::vec3(sideX, bottomWallCenterY, centerZ),
                                              glm::vec3(1.0f, doorHeight, zSpan));
                myCube->draw(lightingShader, mBot, wallColor); // Draw final lower side-wall end segment.
            }
        }

        // 4a. Left Back Wall - Split for Mihrab Opening (with window alcove opening)
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 4.0f, 2.0f);
        {
            // Left back wall spans x: -79 to -15 (width 64), centered at x=-47
            // Window opening: 30 wide (x) x 14 tall (y), centered in the wall segment
            // Sill at 8 units above wall base (kMosqueWallBaseY)
            const float lbwLeft = -79.0f;
            const float lbwRight = -15.0f;
            const float lbwCenterX = (lbwLeft + lbwRight) * 0.5f; // -47
            const float lbwWidth = lbwRight - lbwLeft;            // 64

            const float winWidth = 30.0f;  // x-axis (wider)
            const float winHeight = 14.0f; // y-axis (shorter)
            const float winSillH = 8.0f;   // sill height above wall base

            const float winLeft = lbwCenterX - winWidth * 0.5f;
            const float winRight = lbwCenterX + winWidth * 0.5f;
            const float winBottomY = kMosqueWallBaseY + winSillH;
            const float winTopY = winBottomY + winHeight;

            const float wallBottomY = kMosqueWallBaseY;
            const float wallTopY = kMosqueWallBaseY + kMosqueWallHeight;
            const float backZ = -48.5f;

            // Top strip (full width, above window)
            {
                float h = wallTopY - winTopY;
                float cy = winTopY + h * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(lbwCenterX, cy, backZ), glm::vec3(lbwWidth, h, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Left back-wall top strip above window.
            }
            // Bottom strip / sill (full width, below window)
            {
                float h = winBottomY - wallBottomY;
                float cy = wallBottomY + h * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(lbwCenterX, cy, backZ), glm::vec3(lbwWidth, h, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Left back-wall sill below window.
            }
            // Left pillar (between wall left edge and window left edge, window height)
            {
                float w = winLeft - lbwLeft;
                float cx = lbwLeft + w * 0.5f;
                float cy = winBottomY + winHeight * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(cx, cy, backZ), glm::vec3(w, winHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Left back-wall left pillar beside window.
            }
            // Right pillar (between window right edge and wall right edge, window height)
            {
                float w = lbwRight - winRight;
                float cx = winRight + w * 0.5f;
                float cy = winBottomY + winHeight * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(cx, cy, backZ), glm::vec3(w, winHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Left back-wall right pillar beside window.
            }
        }

        // 4b. Right Back Wall - Split for Mihrab Opening (with window alcove opening)
        {
            glBindTexture(GL_TEXTURE_2D, texWall);
            lightingShader.setVec2("uvTiling", 4.0f, 2.0f);

            // Right back wall spans x: 15 to 79 (width 64), centered at x=47
            const float rbwLeft = 15.0f;
            const float rbwRight = 79.0f;
            const float rbwCenterX = (rbwLeft + rbwRight) * 0.5f; // 47
            const float rbwWidth = rbwRight - rbwLeft;            // 64

            const float winWidth = 30.0f;
            const float winHeight = 14.0f;
            const float winSillH = 8.0f;

            const float winLeft = rbwCenterX - winWidth * 0.5f;
            const float winRight = rbwCenterX + winWidth * 0.5f;
            const float winBottomY = kMosqueWallBaseY + winSillH;
            const float winTopY = winBottomY + winHeight;

            const float wallBottomY = kMosqueWallBaseY;
            const float wallTopY = kMosqueWallBaseY + kMosqueWallHeight;
            const float backZ = -48.5f;

            // Top strip (full width, above window)
            {
                float h = wallTopY - winTopY;
                float cy = winTopY + h * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(rbwCenterX, cy, backZ), glm::vec3(rbwWidth, h, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Right back-wall top strip above window.
            }
            // Bottom strip / sill (full width, below window)
            {
                float h = winBottomY - wallBottomY;
                float cy = wallBottomY + h * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(rbwCenterX, cy, backZ), glm::vec3(rbwWidth, h, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Right back-wall sill below window.
            }
            // Left pillar (between wall left edge and window left edge, window height)
            {
                float w = winLeft - rbwLeft;
                float cx = rbwLeft + w * 0.5f;
                float cy = winBottomY + winHeight * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(cx, cy, backZ), glm::vec3(w, winHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Right back-wall left pillar beside window.
            }
            // Right pillar (between window right edge and wall right edge, window height)
            {
                float w = rbwRight - winRight;
                float cx = winRight + w * 0.5f;
                float cy = winBottomY + winHeight * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(cx, cy, backZ), glm::vec3(w, winHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Right back-wall right pillar beside window.
            }
        }

        // 4c. Alcove bookshelves in back wall window openings
        {
            const float winWidth = 30.0f;
            const float winHeight = 14.0f;
            const float winSillH = 8.0f;
            const float winBottomY = kMosqueWallBaseY + winSillH;
            const float backZ = -48.5f;
            const float alcoveDepth = 4.0f; // how far the bookshelf recesses behind the wall

            // Left back-wall alcove bookshelf (centered at x = -47)
            drawBackWallAlcoveBookshelf(lightingShader, myCube, texWood, texWall, texBook,
                                        mosqueRoot, -47.0f, winWidth, winHeight, winBottomY, backZ, alcoveDepth, wallColor);

            // Right back-wall alcove bookshelf (centered at x = 47)
            drawBackWallAlcoveBookshelf(lightingShader, myCube, texWood, texWall, texBook,
                                        mosqueRoot, 47.0f, winWidth, winHeight, winBottomY, backZ, alcoveDepth, wallColor);
        }

        // 5. Front Wall
        glBindTexture(GL_TEXTURE_2D, texWall);
        lightingShader.setVec2("uvTiling", 8.0f, 2.5f);
        {
            std::vector<float> frontPillarXs = {-30.0f, 5.0f, 40.0f, 75.0f, 110.0f, 145.0f};
            std::vector<float> localPillars;
            for (float px : frontPillarXs)
            {
                float localX = px - 40.0f;
                if (localX >= -78.0f && localX <= 78.0f)
                    localPillars.push_back(localX);
            }

            float doorHeight = 20.0f;
            float totalWallHeight = kMosqueWallHeight;
            float topWallHeight = totalWallHeight - doorHeight;
            float baseWallY = 2.0f;
            float topWallCenterY = baseWallY + doorHeight + (topWallHeight * 0.5f);
            float bottomWallCenterY = baseWallY + (doorHeight * 0.5f);

            float wallLeftEdge = -79.0f;
            float wallRightEdge = 79.0f;

            // 1. Continuous Top Wall
            {
                float totalWidth = wallRightEdge - wallLeftEdge;
                float centerX = (wallLeftEdge + wallRightEdge) * 0.5f;
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(centerX, topWallCenterY, 48.5f), glm::vec3(totalWidth, topWallHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Draw upper front-wall strip above doors.
            }

            // 2. Vertical wall blocks between the lower doors
            float doorWidth = 23.0f;
            std::vector<float> doorGapsLeft;
            std::vector<float> doorGapsRight;

            for (size_t i = 0; i < localPillars.size() - 1; ++i)
            {
                float midX = (localPillars[i] + localPillars[i + 1]) * 0.5f;
                doorGapsLeft.push_back(midX - (doorWidth * 0.5f));
                doorGapsRight.push_back(midX + (doorWidth * 0.5f));
            }

            float currentX = wallLeftEdge;
            for (size_t i = 0; i < doorGapsLeft.size(); ++i)
            {
                float nextDoorStart = doorGapsLeft[i];
                if (nextDoorStart > currentX)
                {
                    float width = nextDoorStart - currentX;
                    float centerX = currentX + (width * 0.5f);
                    glm::mat4 m = composeModel(mosqueRoot, glm::vec3(centerX, bottomWallCenterY, 48.5f), glm::vec3(width, doorHeight, 1.0f));
                    myCube->draw(lightingShader, m, wallColor); // Draw one lower front-wall segment between door openings.
                }
                currentX = doorGapsRight[i];
            }

            if (currentX < wallRightEdge)
            {
                float width = wallRightEdge - currentX;
                float centerX = currentX + (width * 0.5f);
                glm::mat4 m = composeModel(mosqueRoot, glm::vec3(centerX, bottomWallCenterY, 48.5f), glm::vec3(width, doorHeight, 1.0f));
                myCube->draw(lightingShader, m, wallColor); // Draw final lower front-wall end segment.
            }

            // Openable mosque double-door (front)
            lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
            float angle = glm::radians(85.0f * doorOpenAmount);

            float doorY = 12.0f;
            float doorZ = 48.6f;
            float leafW = 11.5f;
            float leafH = 20.0f;
            float leafT = 0.35f;

            std::vector<float> frontDoorCenters = {-52.5f, -17.5f, 17.5f, 52.5f};
            for (float doorCenterX : frontDoorCenters)
            {
                // Left leaf
                glBindTexture(GL_TEXTURE_2D, texDoorLeft);
                float hingeX = doorCenterX - leafW;
                glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(hingeX, doorY, doorZ));
                m = glm::rotate(m, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(leafW * 0.5f, 0.0f, 0.0f));
                m = glm::scale(m, glm::vec3(leafW, leafH, leafT));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw front facade left door leaf.

                // Right leaf
                glBindTexture(GL_TEXTURE_2D, texDoorRight);
                hingeX = doorCenterX + leafW;
                m = glm::translate(mosqueRoot, glm::vec3(hingeX, doorY, doorZ));
                m = glm::rotate(m, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(-leafW * 0.5f, 0.0f, 0.0f));
                m = glm::scale(m, glm::vec3(leafW, leafH, leafT));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw front facade right door leaf.
            }

            // Openable mosque double-doors (right side wall)
            glBindTexture(GL_TEXTURE_2D, texDoorLeft);
            lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
            float angleRight = glm::radians(85.0f * doorOpenAmount);

            float doorX = 78.6f;
            float doorYRight = 10.5f;
            float leafWRight = 8.0f;
            float leafHRight = 16.0f;
            float leafTRight = 0.35f;

            std::vector<float> sideDoorCenters = {-30.0f, 0.0f, 32.0f};
            for (float doorCenterZ : sideDoorCenters)
            {
                // Left leaf
                glBindTexture(GL_TEXTURE_2D, texDoorLeft);
                float hingeZ = doorCenterZ - leafWRight;
                glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(doorX, doorYRight, hingeZ));
                m = glm::rotate(m, -angleRight, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(0.0f, 0.0f, leafWRight * 0.5f));
                m = glm::scale(m, glm::vec3(leafTRight, leafHRight, leafWRight));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw right-side wall left door leaf.

                // Right leaf
                glBindTexture(GL_TEXTURE_2D, texDoorRight);
                hingeZ = doorCenterZ + leafWRight;
                m = glm::translate(mosqueRoot, glm::vec3(doorX, doorYRight, hingeZ));
                m = glm::rotate(m, angleRight, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(0.0f, 0.0f, -leafWRight * 0.5f));
                m = glm::scale(m, glm::vec3(leafTRight, leafHRight, leafWRight));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw right-side wall right door leaf.
            }

            // Openable mosque double-window (left side wall)
            lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
            float angleWin = glm::radians(70.0f * windowOpenAmount);

            float windowX = -78.6f;
            float windowY = kSideWindowY;
            float leafWWin = 8.0f;
            float leafHWin = kSideWindowLeafHeight;
            float leafTWin = 0.3f;

            std::vector<float> sideWindowCenters = {-30.0f, 0.0f, 32.0f};
            for (float windowCenterZ : sideWindowCenters)
            {
                // Left leaf
                glBindTexture(GL_TEXTURE_2D, texWindowLeft);
                float hingeZ = windowCenterZ - leafWWin;
                glm::mat4 m = glm::translate(mosqueRoot, glm::vec3(windowX, windowY, hingeZ));
                m = glm::rotate(m, -angleWin, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(0.0f, 0.0f, leafWWin * 0.5f));
                m = glm::scale(m, glm::vec3(leafTWin, leafHWin, leafWWin));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw left-side wall left window leaf.

                // Right leaf
                glBindTexture(GL_TEXTURE_2D, texWindowRight);
                hingeZ = windowCenterZ + leafWWin;
                m = glm::translate(mosqueRoot, glm::vec3(windowX, windowY, hingeZ));
                m = glm::rotate(m, angleWin, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::translate(m, glm::vec3(0.0f, 0.0f, -leafWWin * 0.5f));
                m = glm::scale(m, glm::vec3(leafTWin, leafHWin, leafWWin));
                myCube->draw(lightingShader, m, glm::vec3(1.0f)); // Draw left-side wall right window leaf.
            }
        }

        // Roof slab + dome + 6 ceiling fans
        drawMosqueRoofDomeAndFans(lightingShader, myCube, myCylinder, mySphere, myCone,
                                  texWall, texDome, mosqueRoot,
                                  roofColor, domeColor, goldColor);

        // Veranda roofs (right then front, matching old ordering)
        // They are drawn here so the mihrab walls texture uvTiling matches the old call-site.
        drawRightVeranda(VerandaStage::Roof, lightingShader, lightCubeShader, myCube, myCylinder, mySphere,
                         texFloor, texWall, mosqueRoot, 0.0f, projection, view, glm::vec3(0.0f), roofColor);
        drawFrontVeranda(VerandaStage::Roof, lightingShader, lightCubeShader, myCube, myCylinder, mySphere,
                         texFloor, texWall, mosqueRoot, 0.0f, projection, view, glm::vec3(0.0f), roofColor);

        // Mihrab walls/roof + mihrab fan
        drawMihrab(MihrabStage::WallsRoofFan, lightingShader, myCube, myCylinder,
                   texFloor, texMihrab, texWall, texDome,
                   mosqueRoot, wallColor, roofColor);

        // Mihrab right-side polished chair with short staircase.
        drawMihrabChairWithStairs(lightingShader, myCube, mosqueRoot);

        // --- Corner bookshelves at back wall (two corners) ---
        {
            const float shelfW = 12.0f;
            const float shelfH = 8.0f;
            const float shelfD = 4.5f;
            const float floorY = 3.0f;      // top of foundation
            const float backWallZ = -47.5f; // slightly in front of back wall

            // Left corner bookshelf
            drawCornerBookshelf(lightingShader, myCube, texWood, texBook,
                                mosqueRoot, -70.0f, floorY, backWallZ, shelfW, shelfH, shelfD);

            // Right corner bookshelf
            drawCornerBookshelf(lightingShader, myCube, texWood, texBook,
                                mosqueRoot, 70.0f, floorY, backWallZ, shelfW, shelfH, shelfD);

            // Rehals on top of each bookshelf
            float rehalY = floorY + shelfH; // sits on top surface
            float rehalZ = backWallZ + shelfD * 0.5f;

            // Left bookshelf: 2 rehals
            drawRehal(lightingShader, myCube, texWood, texBook,
                      mosqueRoot, -73.0f, rehalY, rehalZ, 1.2f);
            drawRehal(lightingShader, myCube, texWood, texBook,
                      mosqueRoot, -67.0f, rehalY, rehalZ, 1.0f);

            // Right bookshelf: 2 rehals
            drawRehal(lightingShader, myCube, texWood, texBook,
                      mosqueRoot, 67.0f, rehalY, rehalZ, 1.2f);
            drawRehal(lightingShader, myCube, texWood, texBook,
                      mosqueRoot, 73.0f, rehalY, rehalZ, 1.0f);
        }

        break;
    }
    case MosqueStage::LightVisuals:
    {
        // Draw hanging bulb spot light visual inside mosque
        glm::vec3 localSpotPos = glm::vec3(0.0f, spotLightPos.y, 0.0f);

        float bulbY = localSpotPos.y;
        float wireLen = kChandelierMainRodTopY - bulbY;
        float wireCenterY = (kChandelierMainRodTopY + bulbY) * 0.5f;

        const float shadeScale = 0.95f; // hemisphere shade scale (replaces hanging bulb sphere)
        const int chandelierCount = 4;
        const float chandelierRingRadius = 3.0f;
        const float chandelierLampY = kChandelierLampY;
        const glm::vec3 chandelierLampColor = pointLightsOn ? glm::vec3(0.95f, 0.86f, 0.70f) : glm::vec3(0.2f, 0.2f, 0.2f);

        lightingShader.use();
        {
            // Anchor the rod at the bulb (bottom) so scaling extends upward.
            glm::mat4 m = mosqueRoot;
            // Move to the bottom attachment (bulb) position
            m = glm::translate(m, glm::vec3(localSpotPos.x, bulbY, localSpotPos.z));
            // Scale the unit cylinder so its half-height becomes (wireLen * 0.5f)
            m = glm::scale(m, glm::vec3(0.03f, wireLen * 4.0f, 0.03f));
            // Translate geometry so the original bottom (y = -1) sits at the model origin before scaling
            m = glm::translate(m, glm::vec3(0.0f, 1.0f, 0.0f));
            myCylinder->draw(lightingShader, m, glm::vec3(1.0f)); // Draw central hanging spotlight wire.
        }

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        glm::vec3 bulbColor = spotLightsOn ? spotLightColor : glm::vec3(0.3f, 0.3f, 0.3f);
        lightCubeShader.setVec3("lightColor", bulbColor);

        // Central hemisphere shade (inverted bowl shape, slightly wider than tall)
        glm::mat4 mShade = glm::translate(mosqueRoot, localSpotPos);
        mShade = glm::rotate(mShade, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        mShade = glm::scale(mShade, glm::vec3(shadeScale, shadeScale * 0.75f, shadeScale));
        lightCubeShader.setMat4("model", mShade);
        mySphere->drawHemisphere(lightCubeShader, mShade, bulbColor);

        // --- Decorative brass rim ring around the hemisphere equator ---
        {
            const glm::vec3 rimColor(0.78f, 0.68f, 0.45f); // brass
            const float rimRadius = shadeScale + 0.03f;    // just outside the hemisphere
            const float rimTube = 0.06f;
            const float rimY = localSpotPos.y; // equator level
            const int rimSegments = 28;

            lightingShader.use();
            for (int i = 0; i < rimSegments; ++i)
            {
                float a0 = (2.0f * (float)M_PI) * ((float)i / (float)rimSegments);
                float a1 = (2.0f * (float)M_PI) * ((float)(i + 1) / (float)rimSegments);

                glm::vec3 p0(std::cos(a0) * rimRadius, rimY, std::sin(a0) * rimRadius);
                glm::vec3 p1(std::cos(a1) * rimRadius, rimY, std::sin(a1) * rimRadius);

                glm::vec3 mid = (p0 + p1) * 0.5f;
                glm::vec3 dir = p1 - p0;
                float segLen = glm::length(dir);
                if (segLen < 1e-6f)
                    continue;
                dir /= segLen;

                glm::vec3 up(0.0f, 1.0f, 0.0f);
                glm::vec3 right = glm::normalize(glm::cross(up, dir));
                glm::vec3 realUp = glm::cross(dir, right);

                glm::mat4 mRim = mosqueRoot;
                mRim = glm::translate(mRim, mid);
                glm::mat4 orient(1.0f);
                orient[0] = glm::vec4(right, 0.0f);
                orient[1] = glm::vec4(dir, 0.0f);
                orient[2] = glm::vec4(realUp, 0.0f);
                mRim = mRim * orient;
                mRim = glm::scale(mRim, glm::vec3(rimTube, segLen * 0.55f, rimTube));
                myCylinder->draw(lightingShader, mRim, rimColor);
            }
        }

        // --- Decorative pendant drop below the hemisphere center ---
        {
            const glm::vec3 metalColor(0.78f, 0.68f, 0.45f);
            const float dropY = localSpotPos.y - shadeScale * 0.75f; // bottom of hemisphere

            lightingShader.use();
            // Small sphere at the bottom
            {
                glm::mat4 mDrop = glm::translate(mosqueRoot, glm::vec3(0.0f, dropY - 0.12f, 0.0f));
                mDrop = glm::scale(mDrop, glm::vec3(0.10f, 0.10f, 0.10f));
                mySphere->draw(lightingShader, mDrop, metalColor);
            }
            // Tiny cone finial pointing down
            {
                glm::mat4 mFinial = glm::translate(mosqueRoot, glm::vec3(0.0f, dropY - 0.28f, 0.0f));
                mFinial = glm::rotate(mFinial, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                mFinial = glm::scale(mFinial, glm::vec3(0.06f, 0.18f, 0.06f));
                myCone->draw(lightingShader, mFinial, metalColor);
            }
        }

        // ============================================================
        // Chandelier arm structure: classic drooping Bézier curve arms
        // ============================================================
        // Design: a decorative finial ball on the central rod, from which
        // 4 graceful arms droop downward, then curve outward and slightly
        // upward to reach each ring lamp. This mimics the look of a
        // traditional hanging chandelier.
        //lightingShader.setFloat("materialTexBlend", 1.0f);
        {
            const glm::vec3 metalColor(0.72f, 0.65f, 0.50f); // warm brass
            const float armTubeRadius = 0.07f;
            const int armCurveSegments = 24; // tessellation quality
            // The Y where arms branch from the central rod
            const float armBranchY = localSpotPos.y + 1.3f;

            // --- Decorative finial sphere on the rod at the branch point ---
            lightingShader.use();
            {
                glm::mat4 mFinial = glm::translate(mosqueRoot, glm::vec3(0.0f, armBranchY, 0.0f));
                mFinial = glm::scale(mFinial, glm::vec3(0.22f, 0.18f, 0.22f));
                mySphere->draw(lightingShader, mFinial, metalColor);
            }
            // Small cone/flared collar below the finial
            {
                glm::mat4 mCollar = glm::translate(mosqueRoot, glm::vec3(0.0f, armBranchY - 0.15f, 0.0f));
                mCollar = glm::scale(mCollar, glm::vec3(0.18f, 0.10f, 0.18f));
                myCone->draw(lightingShader, mCollar, metalColor);
            }

            // Helper lambda: evaluate cubic Bézier at t
            auto bezierCubic = [](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) -> glm::vec3
            {
                float u = 1.0f - t;
                float u2 = u * u;
                float u3 = u2 * u;
                float t2 = t * t;
                float t3 = t2 * t;
                return u3 * p0 + 3.0f * u2 * t * p1 + 3.0f * u * t2 * p2 + t3 * p3;
            };

            for (int i = 0; i < chandelierCount; ++i)
            {
                float a = (2.0f * (float)M_PI) * ((float)i / (float)chandelierCount);
                float cosA = std::cos(a);
                float sinA = std::sin(a);

                // Lamp position (end point of the arm)
                glm::vec3 lampPos(cosA * chandelierRingRadius, chandelierLampY, sinA * chandelierRingRadius);

                // --- Cubic Bézier control points ---
                // The curve sweeps OUTWARD first (clearing the hemisphere shade
                // which has radius ~0.95 below localSpotPos.y), then droops
                // DOWN, and finally curves to the lamp.
                //
                // P0: start at the finial on the rod
                glm::vec3 P0(cosA * 0.20f, armBranchY, sinA * 0.20f);

                // P1: sweep outward past the hemisphere radius and slightly
                //     down — this ensures the curve clears the shade
                float clearY = localSpotPos.y + 0.2f;
                glm::vec3 P1(cosA * 1.5f, clearY, sinA * 1.5f);

                // P2: droop below lamp level at ~75% of ring radius
                float droopY = chandelierLampY - 1.0f;
                glm::vec3 P2(cosA * (chandelierRingRadius * 0.75f), droopY, sinA * (chandelierRingRadius * 0.75f));

                // P3: the lamp position itself
                glm::vec3 P3 = lampPos;

                // Tessellate the curve into oriented cylinder segments
                lightingShader.use();
                for (int seg = 0; seg < armCurveSegments; ++seg)
                {
                    float t0 = (float)seg / (float)armCurveSegments;
                    float t1 = (float)(seg + 1) / (float)armCurveSegments;

                    glm::vec3 pt0 = bezierCubic(P0, P1, P2, P3, t0);
                    glm::vec3 pt1 = bezierCubic(P0, P1, P2, P3, t1);

                    glm::vec3 mid = (pt0 + pt1) * 0.5f;
                    glm::vec3 dir = pt1 - pt0;
                    float segLen = glm::length(dir);
                    if (segLen < 1e-6f)
                        continue;
                    dir /= segLen;

                    // Build orientation: align cylinder Y-axis with segment direction
                    glm::vec3 arbitrary = (std::abs(dir.y) < 0.99f)
                                              ? glm::vec3(0.0f, 1.0f, 0.0f)
                                              : glm::vec3(1.0f, 0.0f, 0.0f);
                    glm::vec3 right = glm::normalize(glm::cross(arbitrary, dir));
                    glm::vec3 realUp = glm::cross(dir, right);

                    glm::mat4 mArm = mosqueRoot;
                    mArm = glm::translate(mArm, mid);
                    glm::mat4 orient(1.0f);
                    orient[0] = glm::vec4(right, 0.0f);
                    orient[1] = glm::vec4(dir, 0.0f);
                    orient[2] = glm::vec4(realUp, 0.0f);
                    mArm = mArm * orient;
                    mArm = glm::scale(mArm, glm::vec3(armTubeRadius, segLen * 0.55f, armTubeRadius));
                    myCylinder->draw(lightingShader, mArm, metalColor);
                }

                // Small decorative sphere at lamp attachment
                {
                    glm::vec3 jointPos = lampPos + glm::vec3(0.0f, 0.40f, 0.0f);
                    glm::mat4 mJoint = glm::translate(mosqueRoot, jointPos);
                    mJoint = glm::scale(mJoint, glm::vec3(0.10f));
                    mySphere->draw(lightingShader, mJoint, metalColor);
                }
            }
        }

        // ============================================================
        // Chandelier ring lamp visuals (hemisphere shades + emissive bodies)
        // ============================================================
        for (int i = 0; i < chandelierCount; ++i)
        {
            float a = (2.0f * (float)M_PI) * ((float)i / (float)chandelierCount);
            glm::vec3 localPosToMosque = glm::vec3(std::cos(a) * chandelierRingRadius, chandelierLampY, std::sin(a) * chandelierRingRadius);

            // Small hemisphere shade above each lamp (inverted bowl)
            lightingShader.use();
            {
                glm::vec3 shadePos = localPosToMosque + glm::vec3(0.0f, 0.38f, 0.0f);
                glm::mat4 mLampShade = glm::translate(mosqueRoot, shadePos);
                mLampShade = glm::rotate(mLampShade, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                mLampShade = glm::scale(mLampShade, glm::vec3(0.30f, 0.16f, 0.30f));
                mySphere->drawHemisphere(lightingShader, mLampShade, glm::vec3(0.72f, 0.65f, 0.50f));
            }

            // Emissive lamp body
            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);

            glm::mat4 mLamp = composeModel(mosqueRoot, localPosToMosque, glm::vec3(0.20f, 0.38f, 0.20f));
            lightCubeShader.setMat4("model", mLamp);
            lightCubeShader.setVec3("lightColor", chandelierLampColor);
            myCylinder->draw(lightCubeShader, mLamp, chandelierLampColor);
        }

        // Draw hanging lantern visuals for the two inside point lights
        for (int i = 6; i <= 7; ++i)
        {
            glm::vec3 localPosToMosque = glm::vec3(i == 6 ? -35.0f : 35.0f, kInsidePendantY, 0.0f);

            glm::vec3 brassColor(0.75f, 0.62f, 0.35f);
            glm::vec3 darkMetal(0.25f, 0.25f, 0.25f);

            lightingShader.use();
            {
                // 1. Suspension rod to ceiling
                float baseWireY = localPosToMosque.y + 0.6f;
                float wireLen = kInteriorCeilingY - baseWireY;
                glm::mat4 mWire = composeModel(mosqueRoot, glm::vec3(localPosToMosque.x, baseWireY + wireLen * 0.5f, localPosToMosque.z), glm::vec3(0.03f, wireLen * 0.5f, 0.03f));
                myCylinder->draw(lightingShader, mWire, darkMetal);

                // 2. Top fixture pieces (Neck and Cone Cap)
                glm::mat4 mNeck = composeModel(mosqueRoot, localPosToMosque + glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.18f, 0.25f, 0.18f));
                myCylinder->draw(lightingShader, mNeck, brassColor);

                glm::mat4 mCap = composeModel(mosqueRoot, localPosToMosque + glm::vec3(0.0f, 0.35f, 0.0f), glm::vec3(0.45f, 0.2f, 0.45f));
                myCone->draw(lightingShader, mCap, brassColor);

                // 3. Ornate Lampshade (Inverted Hemisphere)
                glm::mat4 mShade = glm::translate(mosqueRoot, localPosToMosque + glm::vec3(0.0f, 0.22f, 0.0f));
                mShade = glm::rotate(mShade, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                mShade = glm::scale(mShade, glm::vec3(1.2f, 0.65f, 1.2f));
                mySphere->drawHemisphere(lightingShader, mShade, brassColor);

                // 4. Central rod piercing through the bulb to support the bottom finial
                glm::mat4 mRod = composeModel(mosqueRoot, localPosToMosque - glm::vec3(0.0f, 0.2f, 0.0f), glm::vec3(0.05f, 0.8f, 0.05f));
                myCylinder->draw(lightingShader, mRod, brassColor);

                // 5. Bottom decorative finial
                glm::mat4 mBot = glm::translate(mosqueRoot, localPosToMosque - glm::vec3(0.0f, 0.65f, 0.0f));
                mBot = glm::rotate(mBot, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                mBot = glm::scale(mBot, glm::vec3(0.2f, 0.3f, 0.2f));
                myCone->draw(lightingShader, mBot, brassColor);

                glm::mat4 mDrop = composeModel(mosqueRoot, localPosToMosque - glm::vec3(0.0f, 0.95f, 0.0f), glm::vec3(0.12f, 0.12f, 0.12f));
                mySphere->draw(lightingShader, mDrop, brassColor);
            }

            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);

            // The emissive lamp bulb itself
            glm::mat4 mLamp = composeModel(mosqueRoot, localPosToMosque - glm::vec3(0.0f, 0.15f, 0.0f), glm::vec3(0.6f, 0.65f, 0.6f));
            lightCubeShader.setMat4("model", mLamp);
            glm::vec3 c = pointLightsOn ? glm::vec3(1.0f, 0.95f, 0.85f) : glm::vec3(0.2f, 0.2f, 0.2f);
            lightCubeShader.setVec3("lightColor", c);
            mySphere->draw(lightCubeShader, mLamp, c); // Draw one inside pendant lamp bulb.
        }

        break;
    }
    }
}

// This function manages the drawing sequence for the entire outdoor and indoor
// scene for a single viewport. It sets up lighting uniforms, then calls all the
// specific draw functions (ground, trees, mosque, wadhu, etc.) in a structured hierarchy.
void drawScene(Shader &lightingShader, Shader &lightCubeShader,
               glm::mat4 projection, glm::mat4 view, glm::vec3 camPos,
               glm::vec3 wallColor, glm::vec3 domeColor, glm::vec3 minaretColor,
               glm::vec3 roofColor, glm::vec3 doorColor, glm::vec3 rimColor,
               glm::vec3 pillarColor,
               glm::vec3 waterColor, glm::vec3 goldColor, glm::vec3 tileColor,
               PointLight *pointLights[], DirLight *dirLight, SpotLight *spotLight,
               glm::vec3 lampLightColor, glm::vec3 spotLightPos, glm::vec3 spotLightColor,
               glm::vec3 pondPos, float pondW, float pondD, float pondWallH, float pondWallT,
               glm::vec3 pondCornerPositions[], glm::vec3 pondCornerColors[],
               int numLampPairs, float lampFirst, float lampStep, float lampXOffset,
               int numRightLampPairs, float rightLampFirst, float rightLampStep, float rightLampZOffset)
{
    float mosqueZ = -80.0f;

    const glm::mat4 sceneRoot = glm::mat4(1.0f);
    const glm::mat4 mosqueRoot = glm::translate(sceneRoot, glm::vec3(40.0f, 0.0f, mosqueZ - 30.0f));
    // Start along the front veranda line
    const glm::mat4 leftWadhuRoot = glm::translate(mosqueRoot, glm::vec3(-99.0f, 0.0f, 100.0f));

    // Hierarchical scaling for Wadhu structures
    glm::mat4 wadhuScale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust this to scale the entire wadhu building
    glm::mat4 leftWadhuTransform = leftWadhuRoot * wadhuScale;
    syncFaucetSources(leftWadhuTransform);

    // ========== USE LIGHTING SHADER FOR SCENE ==========
    lightingShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);
    lightingShader.setVec3("viewPos", camPos);
    lightingShader.setVec2("uvTiling", 1.0f, 1.0f);
    lightingShader.setFloat("materialTexBlend", 0.0f);

    // Toggle uniforms
    lightingShader.setBool("dirLightOn", dirLightOn);
    lightingShader.setBool("pointLightsOn", pointLightsOn);
    lightingShader.setBool("spotLightsOn", spotLightsOn);
    lightingShader.setBool("ambientOn", ambientOn);
    lightingShader.setBool("diffuseOn", diffuseOn);
    lightingShader.setBool("specularOn", specularOn);

    // --- Directional light (sun from above) ---
    dirLight->setUpDirLight(lightingShader);

    // --- Point lights (8 lamp posts) ---
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        pointLights[i]->setUpPointLight(lightingShader);
    }

    // --- Spot light: single hanging bulb inside mosque ---
    spotLight->setUpSpotLight(lightingShader);

    // Zero out unused spot lights to prevent artifacts
    for (int i = 1; i < 4; ++i)
    {
        std::string base = "spotLights[" + std::to_string(i) + "].";
        lightingShader.setVec3(base + "position", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3(base + "direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setFloat(base + "cutOff", glm::cos(glm::radians(1.0f)));
        lightingShader.setVec3(base + "ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3(base + "diffuse", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3(base + "specular", 0.0f, 0.0f, 0.0f);
        lightingShader.setFloat(base + "constant", 1.0f);
        lightingShader.setFloat(base + "linear", 0.0f);
        lightingShader.setFloat(base + "quadratic", 0.0f);
    }

    drawGround(lightingShader, myCube, sceneRoot);
    drawTrees(lightingShader, myCylinder, mySphere, sceneRoot);

    // Foundations (main base + veranda bases + mihrab base) - before Wadhu.
    drawMainMosque(MosqueStage::Foundations,
                   lightingShader, lightCubeShader,
                   myCube, myCylinder, mySphere, myCone,
                   texFloor, texWall, texDome, texMihrab,
                   mosqueRoot,
                   projection, view,
                   wallColor, domeColor, roofColor, goldColor,
                   spotLightPos, spotLightColor);

    drawFrontVeranda(VerandaStage::Base,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    drawRightVeranda(VerandaStage::Base,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    drawMihrab(MihrabStage::Base,
               lightingShader,
               myCube, myCylinder,
               texFloor, texMihrab, texWall, texDome,
               mosqueRoot,
               wallColor, roofColor);

    // --------------- WADHU (full group) ---------------
    drawWadhu(
        lightingShader, lightCubeShader,
        myCube, myCylinder, mySphere,
        leftWadhuTransform,
        projection, view,
        wallColor, tileColor);
    renderFaucetDroplets(lightingShader, mySphere);

    // =====================================================
    // Hierarchical build: mosque + verandas + environment
    // (Main remaining render sequence.)
    // =====================================================
    drawMainMosque(MosqueStage::WallsAndRoof,
                   lightingShader, lightCubeShader,
                   myCube, myCylinder, mySphere, myCone,
                   texFloor, texWall, texDome, texMihrab,
                   mosqueRoot,
                   projection, view,
                   wallColor, domeColor, roofColor, goldColor,
                   spotLightPos, spotLightColor);

    drawFrontVeranda(VerandaStage::Pillars,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    drawRightVeranda(VerandaStage::Pillars,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    drawMinarets(lightingShader, myCube, myCylinder, mySphere, myCone,
                 texWall, mosqueRoot,
                 minaretColor, roofColor, wallColor, domeColor, goldColor);

    drawWalkwayAndStairs(
        lightingShader, myCube, myCylinder,
        texRoad, texWall, texFloor,
        sceneRoot, mosqueRoot,
        numLampPairs, lampFirst, lampStep, lampXOffset,
        tileColor,
        numRightLampPairs, rightLampFirst, rightLampStep, rightLampZOffset);

    drawPond(lightingShader, myCube, sceneRoot, glm::vec3(15.0f, 0.0f, 60.0f), 24.0f, 48.0f, pondWallH, pondWallT, waterColor, rimColor);
    drawPond(lightingShader, myCube, sceneRoot, glm::vec3(65.0f, 0.0f, 60.0f), 24.0f, 48.0f, pondWallH, pondWallT, waterColor, rimColor);

    // Road lamp heads (only) - mosque + veranda lights are built with their own functions below.
    drawRoadLampHeadVisuals(lightCubeShader, mySphere,
                            projection, view,
                            sceneRoot, lampLightColor,
                            numLampPairs, lampFirst, lampStep, lampXOffset,
                            numRightLampPairs, rightLampFirst, rightLampStep, rightLampZOffset);

    drawMainMosque(MosqueStage::LightVisuals,
                   lightingShader, lightCubeShader,
                   myCube, myCylinder, mySphere, myCone,
                   texFloor, texWall, texDome, texMihrab,
                   mosqueRoot,
                   projection, view,
                   wallColor, domeColor, roofColor, goldColor,
                   spotLightPos, spotLightColor);

    drawFrontVeranda(VerandaStage::Lamps,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    drawRightVeranda(VerandaStage::Lamps,
                     lightingShader, lightCubeShader,
                     myCube, myCylinder, mySphere,
                     texFloor, texWall,
                     mosqueRoot, mosqueZ,
                     projection, view,
                     pillarColor, roofColor);

    // End of viewport draw.
}

// This function handles all keyboard input during the render loop. It processes
// camera movement, rotation, orbit toggling, light toggling, shading toggling,
// and triggers the interactive animations (doors, windows, bookshelves, water).
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- Viewport switching with TAB ---
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!tabKeyPressed)
        {
            activeViewport = (activeViewport + 1) % 4;
            tabKeyPressed = true;
            const char *names[] = {"Isometric", "Top", "Front", "Inside"};
            std::cout << "Active Viewport: " << names[activeViewport] << std::endl;
        }
    }
    else
    {
        tabKeyPressed = false;
    }

    Camera &camera = cameras[activeViewport];

    // --- Movement (WASD + E/R) ---
    float baseSpeed = camera.MovementSpeed;
    camera.MovementSpeed = baseSpeed * 4.5f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    camera.MovementSpeed = baseSpeed;

    // --- Rotation (X, Y, Z) with Shift to reverse direction ---
    float rotSpeedDegPerSec = 45.0f;
    float rotThisFrame = rotSpeedDegPerSec * deltaTime;
    bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        float pitchOffset = shift ? -rotThisFrame : rotThisFrame;
        camera.ProcessRotation(pitchOffset, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        float yawOffset = shift ? -rotThisFrame : rotThisFrame;
        camera.ProcessRotation(0.0f, yawOffset, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        float rollOffset = shift ? -rotThisFrame : rotThisFrame;
        camera.ProcessRotation(0.0f, 0.0f, rollOffset);
    }

    // --- Orbit mode toggle (F) ---
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fKeyPressed)
        {
            orbitMode = !orbitMode;
            fKeyPressed = true;

            if (orbitMode)
            {
                orbitCenter = glm::vec3(0.0f, 0.0f, 6.0f);
                orbitRadius = glm::length(camera.Position - orbitCenter);
                glm::vec3 dir = glm::normalize(camera.Position - orbitCenter);
                orbitAngle = atan2(dir.z, dir.x);
            }
        }
    }
    else
    {
        fKeyPressed = false;
    }

    // --- Execute orbit if active ---
    if (orbitMode)
    {
        orbitAngle += orbitSpeed * deltaTime;

        glm::vec3 newPos;
        newPos.x = orbitCenter.x + orbitRadius * cos(orbitAngle);
        newPos.z = orbitCenter.z + orbitRadius * sin(orbitAngle);
        newPos.y = camera.Position.y;

        camera.Position = newPos;
        camera.LookAt(orbitCenter);
    }

    // --- Light type toggles ---
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (!key1Pressed)
        {
            dirLightOn = !dirLightOn;
            key1Pressed = true;
            std::cout << "Directional Light: " << (dirLightOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key1Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (!key2Pressed)
        {
            pointLightsOn = !pointLightsOn;
            key2Pressed = true;
            std::cout << "Point Lights: " << (pointLightsOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key2Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (!key3Pressed)
        {
            spotLightsOn = !spotLightsOn;
            key3Pressed = true;
            std::cout << "Spot Lights: " << (spotLightsOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key3Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (!key4Pressed)
        {
            fansOn = !fansOn;
            key4Pressed = true;
            std::cout << "Mosque Fans: " << (fansOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key4Pressed = false;
    }

    // --- Light component toggles ---
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        if (!key5Pressed)
        {
            ambientOn = !ambientOn;
            key5Pressed = true;
            std::cout << "Ambient: " << (ambientOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key5Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
    {
        if (!key6Pressed)
        {
            diffuseOn = !diffuseOn;
            key6Pressed = true;
            std::cout << "Diffuse: " << (diffuseOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key6Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
    {
        if (!key7Pressed)
        {
            specularOn = !specularOn;
            key7Pressed = true;
            std::cout << "Specular: " << (specularOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        key7Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        if (!keyOPressed)
        {
            doorsOpen = !doorsOpen;
            keyOPressed = true;
            std::cout << "Mosque Door: " << (doorsOpen ? "OPEN" : "CLOSED") << std::endl;
        }
    }
    else
    {
        keyOPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!keyPPressed)
        {
            windowsOpen = !windowsOpen;
            keyPPressed = true;
            std::cout << "Mosque Window: " << (windowsOpen ? "OPEN" : "CLOSED") << std::endl;
        }
    }
    else
    {
        keyPPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        if (!keyHPressed)
        {
            faucetWaterOn = !faucetWaterOn;
            keyHPressed = true;
            std::cout << "Faucet Water: " << (faucetWaterOn ? "ON" : "OFF") << std::endl;
        }
    }
    else
    {
        keyHPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        if (!keyBPressed)
        {
            bookshelvesOpen = !bookshelvesOpen;
            keyBPressed = true;
            std::cout << "Corner Bookshelves: " << (bookshelvesOpen ? "OPEN" : "CLOSED") << std::endl;
        }
    }
    else
    {
        keyBPressed = false;
    }

    // --- Shading toggle (Phong <-> Gouraud) ---
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (!gKeyPressed)
        {
            gouraudShading = !gouraudShading;
            gKeyPressed = true;
            std::cout << "Shading: " << (gouraudShading ? "GOURAUD" : "PHONG") << std::endl;
        }
    }
    else
    {
        gKeyPressed = false;
    }
}

// This callback function handles window resize events to adjust the viewport if needed.
// (Currently does nothing as viewport is managed dynamically per viewport area).
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Don't set glViewport here since we manage it per-viewport in render loop
}

// This callback function processes mouse movement input to adjust the camera's
// look direction (pitch and yaw).
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
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

    cameras[activeViewport].ProcessMouseMovement(xoffset, yoffset);
}

// This callback function processes mouse scroll wheel input to adjust the
// active camera's zoom level (Field of View).
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    cameras[activeViewport].ProcessMouseScroll(static_cast<float>(yoffset));
}

// Returns a perspective projection matrix based on Field of View, aspect ratio,
// near plane and far plane. Acts as a custom replacement for glm::perspective.
glm::mat4 myPerspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    float tanHalfFov = tan(fovRadians / 2.0f);

    glm::mat4 result(0.0f);
    result[0][0] = 1.0f / (aspect * tanHalfFov);
    result[1][1] = 1.0f / tanHalfFov;
    result[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result[2][3] = -1.0f;
    result[3][2] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    return result;
}

// Returns an orthographic projection matrix for 2D or isometric-style flat rendering.
// Acts as a custom replacement for glm::ortho.
glm::mat4 myOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    glm::mat4 result(0.0f);
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = -2.0f / (farPlane - nearPlane);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result[3][3] = 1.0f;
    return result;
}

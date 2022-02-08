#define WIN32_LEAN_AND_MEAN
#define GLM_CONSTEXPR_SIMD

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include <chrono>
#include <array>

#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "BufferLayout.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "ParticleEmitter.hpp"



std::uint32_t MouseX = 0;
std::uint32_t MouseY = 0;

int WindowWidth = 0;
int WindowHeight = 0;

std::function<void(void)> leftMouseButtonClickedCallback;
std::function<void(void)> rightMouseButtonClickedCallback;


void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
        {
            std::cerr << message << "\n";

            __debugbreak();
            break;
        };

        default:
            break;
    };
};

void GLFWErrorCallback(int, const char* err_str)
{
    std::cerr << "GLFW Error: " << err_str << "\n";
};


GLFWwindow* InitializeGLFWWindow(int windowWidth, int windowHeight, std::string_view windowTitle)
{
    glfwInit();

    glfwSetErrorCallback(GLFWErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* glfwWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle.data(), nullptr, nullptr);

    WindowWidth = windowWidth;
    WindowHeight = windowHeight;

    glfwMakeContextCurrent(glfwWindow);

    // Draw as fast as computerly possible
    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height)
    {
        WindowWidth = width;
        WindowHeight = height;

        glViewport(0, 0, width, height);
    });

    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow*, int mouseButton, int action, int modifierBits)
    {
        if((mouseButton == GLFW_MOUSE_BUTTON_LEFT) &&
           (action == GLFW_PRESS))
        {
            if(leftMouseButtonClickedCallback != nullptr)
            {
                leftMouseButtonClickedCallback();
            };
        }
        else if((mouseButton == GLFW_MOUSE_BUTTON_RIGHT) &&
                (action == GLFW_PRESS))
        {
            if(rightMouseButtonClickedCallback != nullptr)
            {
                rightMouseButtonClickedCallback();
            };
        };

    });


    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow*, double mouseX, double mouseY)
    {
        MouseX = static_cast<std::uint32_t>(mouseX);
        MouseY = static_cast<std::uint32_t>(mouseY);
    });

    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glfwShowWindow(glfwWindow);

    return glfwWindow;
};


void SetupOpenGL()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLDebugCallback, nullptr);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
};


int main()
{
    
    constexpr std::uint32_t initialWindowWidth = 800;
    constexpr std::uint32_t initialWindowHeight = 600;

    constexpr bool generateEmitters = true;
    constexpr std::uint32_t emittersToGenerate = 700;


    constexpr std::uint32_t particlesPerEmitter = 250;


    // Create a window
    GLFWwindow* glfwWindow = InitializeGLFWWindow(initialWindowWidth, initialWindowHeight,
                                                  "OpenGL - Particle emmiter");



    // Setup "boilerplate" GL functionality
    SetupOpenGL();


    // The main VAO that will be used by the particle emmiter
    VertexArray particleVAO = VertexArray();


    // Particle vertex position, along with texture coordinates
    constexpr float vertexPositions[] =
    {
        // Bottom left
        -1.0f, -1.0f,  0.0f, 0.0f,
        // Bottom right
        1.0f,  -1.0f,  1.0f, 0.0f,
        // Top right
        1.0f,   1.0f,  1.0f, 1.0f,

        // Top right
        1.0f,   1.0f,  1.0f, 1.0f,
        // Top left
        -1.0f,  1.0f,  0.0f, 1.0f,
        // Bottom left
        -1.0f, -1.0f,  0.0f, 0.0f,
    };



    // vertex positions VBO
    VertexBuffer vertexPositionVBO = VertexBuffer(&vertexPositions, sizeof(vertexPositions));

    BufferLayout vertexPositionBufferlayout;


    // Vertex position
    vertexPositionBufferlayout.AddElement<float>(0, 2);

    // Texture coordinate
    vertexPositionBufferlayout.AddElement<float>(1, 2);

    particleVAO.AddBuffer(vertexPositionVBO, vertexPositionBufferlayout);



    // Particle opacity
    VertexBuffer opacityVBO = VertexBuffer(nullptr, sizeof(float) * particlesPerEmitter, GL_DYNAMIC_DRAW);


    BufferLayout opacityBufferLayout;

    opacityBufferLayout.AddElement<float>(2, 1, 1);

    particleVAO.AddBuffer(opacityVBO, opacityBufferLayout);



    std::array<std::uint32_t, particlesPerEmitter> textureUnitBuffer;

    for(std::size_t i = 0; i < textureUnitBuffer.size(); i++)
    {
        textureUnitBuffer[i] = i % 3;
    };


    // Particle texture units
    VertexBuffer particleTextureUnits = VertexBuffer(textureUnitBuffer.data(), sizeof(textureUnitBuffer));

    BufferLayout particleTextureUnitsBufferLayout;

    particleTextureUnitsBufferLayout.AddElement<std::uint32_t>(3, 1, 1);

    particleVAO.AddBuffer(particleTextureUnits, particleTextureUnitsBufferLayout);



    // Particle transforms
    constexpr float particleScaleFactor = 0.05f;

    const glm::mat4 particleTransfrom = glm::scale(glm::mat4(1.0f), { particleScaleFactor, particleScaleFactor, particleScaleFactor });

    VertexBuffer transformVBO = VertexBuffer(nullptr, sizeof(particleTransfrom) * particlesPerEmitter, GL_DYNAMIC_DRAW);

    BufferLayout transformBufferlayout;

    transformBufferlayout.AddElement<float>(4, 4, 1);
    transformBufferlayout.AddElement<float>(5, 4, 1);
    transformBufferlayout.AddElement<float>(6, 4, 1);
    transformBufferlayout.AddElement<float>(7, 4, 1);

    particleVAO.AddBuffer(transformVBO, transformBufferlayout);




    const Texture particleTexture1 = Texture("Resources\\Particle1.png");
    const Texture particleTexture2 = Texture("Resources\\Particle2.png");
    const Texture particleTexture3 = Texture("Resources\\Particle3.png");



    const std::vector<const Texture*> particleTextures =
    {
        &particleTexture1,
        &particleTexture2,
        &particleTexture3,
    };


    // A shader program that will be used by the Particle emitter
    const ShaderProgram texturedShaderProgram = ShaderProgram("ParticleVertexShader.glsl", "ParticleFragmentShader.glsl");

    const ComputeShaderProgram computeShader = ComputeShaderProgram("ParticleTransformShader.glsl");



    // A list of particle emmiters
    std::vector<ParticleEmmiter> particleEmmiters = std::vector<ParticleEmmiter>();



    if constexpr(generateEmitters == false)
    {
        // Add a new particle emmiter on the mouse's position
        leftMouseButtonClickedCallback = [&]()
        {
            const auto mouseNDC = MouseToNDC() / particleScaleFactor;

            particleEmmiters.emplace_back(particlesPerEmitter,
                                          particleScaleFactor,
                                          // Translate the original particle transform to Mouse position
                                          glm::translate(particleTransfrom, { mouseNDC.x, mouseNDC.y, 0 }),
                                          texturedShaderProgram,
                                          particleVAO,
                                          particleTextures,
                                          vertexPositionVBO,
                                          transformVBO,
                                          opacityVBO,
                                          computeShader);
        };


        // Destory all particle emitters
        rightMouseButtonClickedCallback = [&]()
        {
            if(particleEmmiters.empty() == true)
                return;

            particleEmmiters.erase(particleEmmiters.end() - 1);

            /*
            for(ParticleEmmiter& particleEmmiter : particleEmmiters)
            {
                particleEmmiter.Destory();
            };
            */
        };

    }
    else if constexpr(generateEmitters == true)
    {
        std::mt19937 rng = std::mt19937(std::random_device {}());

        const std::uniform_int_distribution particleXDistribution = std::uniform_int_distribution(0, WindowWidth);
        const std::uniform_int_distribution particleYDistribution = std::uniform_int_distribution(0, WindowHeight);

        for(std::size_t i = 0; i < emittersToGenerate; i++)
        {
            const auto emitterPosition = ScreenToNDC({ particleXDistribution(rng), particleYDistribution(rng) }) / particleScaleFactor;

            particleEmmiters.emplace_back(particlesPerEmitter,
                                          particleScaleFactor,
                                          glm::translate(particleTransfrom, { emitterPosition.x, emitterPosition.y, 0 }),
                                          texturedShaderProgram,
                                          particleVAO,
                                          particleTextures,
                                          vertexPositionVBO,
                                          transformVBO,
                                          opacityVBO,
                                          computeShader);
        };
    };




    std::chrono::steady_clock::time_point timePoint1;
    std::chrono::steady_clock::time_point timePoint2;

    // Elapsed time between frames
    std::chrono::duration<float> elapsedTime = std::chrono::duration<float>(0);

    // Number of frames elapsed
    std::uint32_t elapsedFrames = 0;

    // Time change between timePoint2 and timePoint1
    std::chrono::duration<float> delta = {};


    // How often to display FPS
    constexpr auto fpsDisplayInterval = std::chrono::milliseconds(700);


    while(glfwWindowShouldClose(glfwWindow) == false)
    {
        timePoint1 = std::chrono::steady_clock::now();

        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Bind, update, and draw, particles
        auto iterator = particleEmmiters.begin();

        while(iterator != particleEmmiters.cend())
        {
            ParticleEmmiter& particleEmmiter = *iterator;

            // If an emitter was destroyed...
            if(particleEmmiter.GetDestroyed() == true)
            {
                // Remove from emitters list, and update iterator
                iterator = particleEmmiters.erase(iterator);
                continue;
            };

            particleEmmiter.Bind();
            particleEmmiter.Update(delta.count());

            particleEmmiter.Draw();


            iterator++;
        };



        glfwSwapBuffers(glfwWindow);

        timePoint2 = std::chrono::steady_clock::now();

        delta = timePoint2 - timePoint1;
        elapsedTime += std::chrono::duration<float>(delta);
        elapsedFrames++;

        // If enough time has elapsed...
        if(elapsedTime > fpsDisplayInterval)
        {
            float fps = elapsedFrames / elapsedTime.count();

            elapsedFrames = 0;
            elapsedTime = std::chrono::milliseconds(0);

            char tileBuffer[64] { 0 };


            sprintf_s(tileBuffer, sizeof(tileBuffer), "Emmiters: %d, Particles: %d, FPS: %.2f", static_cast<int>(particleEmmiters.size()), static_cast<int>(particleEmmiters.size() * particlesPerEmitter), fps);

            // Display FPS
            glfwSetWindowTitle(glfwWindow, tileBuffer);
        };

    };


    glfwDestroyWindow(glfwWindow);
};
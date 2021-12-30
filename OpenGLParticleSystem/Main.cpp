#define WIN32_LEAN_AND_MEAN
#define GLM_CONSTEXPR_SIMD

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include <chrono>
#include <functional>

#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "BufferLayout.hpp"
#include "Math.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"



std::uint32_t MouseX = 0;
std::uint32_t MouseY = 0;

int WindowWidth = 0;
int WindowHeight = 0;

std::function<void(void)> leftMouseButtonClickedCallback;
std::function<void(void)> rightMouseButtonClickedCallback;


void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    switch (severity)
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
        if ((mouseButton == GLFW_MOUSE_BUTTON_LEFT) &&
            (action == GLFW_PRESS))
        {
            leftMouseButtonClickedCallback();
        }
        else if ((mouseButton == GLFW_MOUSE_BUTTON_RIGHT) &&
                 (action == GLFW_PRESS))
        {
            rightMouseButtonClickedCallback();
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



/// <summary>
/// A simple parabolic trajectory function, returns the next 'y' position of a particle depending on it's 'x' position
/// </summary>
/// <param name="particleX">  </param>
/// <param name="a"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <param name="b"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <returns></returns>
constexpr float ParticleTrajectoryFunction(const float particleX, float a = 1.0f, float b = 1.0f)
{
    return -a * (particleX * particleX) + (b * particleX);
};


struct Particle
{
    float TrajectoryA = 0.0f;
    float TrajectoryB = 0.0f;

    glm::vec2 Trajectory = glm::vec2(1.0f);

    float Rate = 0.0f;

    glm::mat4 Transform = glm::mat4(1.0f);

    float Opacity = 0.0f;

    float OpacityDecreaseRate = 0.0f;
};


class ParticleEmmiter
{
private:

    std::reference_wrapper<const ShaderProgram> _shaderProgram;
    std::reference_wrapper<std::mt19937> _rng;

    std::reference_wrapper<const std::uniform_real_distribution<float>> _rateDistribution;
    std::reference_wrapper<const std::uniform_real_distribution<float>> _trajectoryADistribution;
    std::reference_wrapper<const std::uniform_real_distribution<float>> _trajectoryBDistribution;
    std::reference_wrapper<const std::uniform_real_distribution<float>> _opacityDecreaseRateDistribution;


    /// <summary>
    /// A transform that will be applied for every particle at the moment the 'Update()' function is called.
    /// Can be thought of as the "View-Transform" 
    /// </summary>
    glm::mat4 _particleEmmiterTransform;

    /// <summary>
    /// A particle transform that will be applied to a given particle after it is reset.
    /// </summary>
    glm::mat4 _particleTransform;


    float _particleScaleFactor;

    std::reference_wrapper<const VertexArray> _particleVAO;

    std::reference_wrapper<const Texture> _particleTexture;

    std::reference_wrapper<const VertexBuffer> _particleVertexPositionVBO;
    std::reference_wrapper<const VertexBuffer> _particleTransformVBO;
    std::reference_wrapper<const VertexBuffer> _particleOpacityVBO;

    std::vector<Particle> _particles;

    /// <summary>
    /// A boolean flag that indicates if this emitter is in the process of being destroyed
    /// </summary>
    bool _desrtoyRequested = false;

public:

    ParticleEmmiter(const std::uint32_t numberOfParticles,
                    const float particleScaleFactor,
                    const glm::mat4& particleEmitterTransform,
                    const ShaderProgram& shaderProgram,
                    const VertexArray& particleVAO,
                    const Texture& texture,
                    const VertexBuffer& particleVertexPositionVBO,
                    const VertexBuffer& particleTransformVBO,
                    const VertexBuffer& particleOpacityVBO,
                    std::mt19937& rng,
                    const std::uniform_real_distribution<float>& rateDistribution,
                    const std::uniform_real_distribution<float>& trajectoryADistribution,
                    const std::uniform_real_distribution<float>& trajectoryBDistribution,
                    const std::uniform_real_distribution<float>& opacityDecreaseRateDistribution) :
        _shaderProgram(shaderProgram),
        _rng(rng),
        _rateDistribution(rateDistribution),
        _trajectoryADistribution(trajectoryADistribution),
        _trajectoryBDistribution(trajectoryBDistribution),
        _opacityDecreaseRateDistribution(opacityDecreaseRateDistribution),
        _particleEmmiterTransform(particleEmitterTransform),
        _particleTransform(glm::mat4(1.0f)),
        _particleScaleFactor(particleScaleFactor),
        _particleVAO(particleVAO),
        _particleTexture(texture),
        _particleVertexPositionVBO(particleVertexPositionVBO),
        _particleTransformVBO(particleTransformVBO),
        _particleOpacityVBO(particleOpacityVBO)
    {
        _particles.resize(numberOfParticles);


        for (std::size_t index = 0;
             Particle & particle : _particles)
        {
            InitializeParticleValues(particle, index);
            index++;
        };

    };


public:

    void Bind() const
    {
        _shaderProgram.get().Bind();
        // _shaderProgram.get().SetInt("WindowWidth", WindowWidth);
        // _shaderProgram.get().SetInt("WindowHeight", WindowHeight);

        _particleVAO.get().Bind();

        _particleTexture.get().Bind();

        _particleVertexPositionVBO.get().Bind();
        _particleTransformVBO.get().Bind();
    };

    void Update(const float deltaTime)
    {
        // TODO: Move matrix math to shaders

        _particleTransformVBO.get().Bind();
        glm::mat4* particleTransformBuffer = reinterpret_cast<glm::mat4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

        _particleOpacityVBO.get().Bind();
        float* particleOpacityBuffer = reinterpret_cast<float*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

        auto iterator = _particles.begin();

        // When using a for-loop and requesting the Emmiter to be destroyed, particles sometimes flicker.
        // Using iterators fixes it, I'm guessing it's do to with indexing(?)
        std::size_t index = 0;
        while (iterator != _particles.cend())
        {
            Particle& particle = *iterator;
            constexpr float rateIncrease = 0.01f;

            // Negative
            if (std::signbit(particle.Rate) == true)
                particle.Rate -= rateIncrease;
            // Positive
            else
                particle.Rate += rateIncrease;


            particle.Trajectory.x += particle.Rate * deltaTime;
            particle.Trajectory.y = ParticleTrajectoryFunction(particle.Trajectory.x, particle.TrajectoryA, particle.TrajectoryB);

            particle.Opacity -= particle.OpacityDecreaseRate * deltaTime;

            // Copy new opacity value into VBO
            std::memcpy(particleOpacityBuffer + index, &particle.Opacity, sizeof(particle.Opacity));


            const glm::vec2 ndcPosition = CartesianToNDC({ particle.Trajectory.x, particle.Trajectory.y }) / _particleScaleFactor;

            
            // First apply the transform which translates the particle onto some screen position
            const glm::mat4 screenTransfrom = glm::translate(_particleEmmiterTransform, { ndcPosition.x , ndcPosition.y, 0.0f })
                // Apply active particle transform
                * particle.Transform;

            // Get translation components of the transform. 
            // This works as long as we don't use non-uniform transformations 
            const glm::vec3 screenPosition = glm::vec3(screenTransfrom[3]);


            // Copy particle-screen transform into VBO
            std::memcpy(particleTransformBuffer + index, glm::value_ptr(screenTransfrom), sizeof(screenTransfrom));


            // If the particle is outside screen bounds..
            if ((screenPosition.y < -1.0f) ||
                // Or if the particle is practically inivsible
                (particle.Opacity < 0.0f))
            {
                if (_desrtoyRequested == true)
                {
                    // After removing the particle, update the iterator and move to the next particle 
                    iterator = _particles.erase(iterator);
                    continue;
                };

                // "Reset" the particle
                InitializeParticleValues(particle, index);
            };

            index++;
            iterator++;
        };


        _particleTransformVBO.get().Bind();
        glUnmapBuffer(GL_ARRAY_BUFFER);

        _particleOpacityVBO.get().Bind();
        glUnmapBuffer(GL_ARRAY_BUFFER);
    };

    void Draw() const
    {
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<std::uint32_t>(_particles.size()));
    };

    void Destory()
    {
        _desrtoyRequested = true;
    };


    glm::mat4& GetParticleEmmiterTransform()
    {
        return _particleEmmiterTransform;
    };

    glm::mat4& GetParticleTransform()
    {
        return _particleTransform;
    };


    bool GetDestroyed() const
    {
        if (_desrtoyRequested == true)
        {
            return _particles.empty();
        };

        return false;
    };


private:

    void InitializeParticleValues(Particle& particle, const std::size_t particleIndex)
    {
        const float newTrajectoryA = _trajectoryADistribution(_rng.get());

        // A very simple way of creating some trajectory variation
        const float newTrajectoryB = (particleIndex & 1) == 1 ?
            -_trajectoryBDistribution(_rng.get()) :
            _trajectoryBDistribution(_rng.get());

        const float newOpacityDecreaseRate = _opacityDecreaseRateDistribution(_rng.get());

        // Correct the rate depending on trajectory
        const float newRate = std::signbit(newTrajectoryB) == true ?
            // "Left" trajectory 
            -_rateDistribution(_rng.get()) :
            // "Right" trajectory
            _rateDistribution(_rng.get());


        particle.TrajectoryA = newTrajectoryA;
        particle.TrajectoryB = newTrajectoryB;

        particle.Trajectory = glm::vec2(0.0f);
        particle.Rate = newRate;

        particle.Opacity = 1.0f;
        particle.OpacityDecreaseRate = newOpacityDecreaseRate;


        // Apply custom particle transform
        particle.Transform = _particleTransform;
    };

};



int main()
{
    constexpr std::uint32_t initialWindowWidth = 800;
    constexpr std::uint32_t initialWindowHeight = 600;

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


    // A VBO for the vertex positions
    VertexBuffer vertexPositionVBO = VertexBuffer(&vertexPositions, sizeof(vertexPositions));

    BufferLayout vertexPositionBufferlayout;

    // Vertex position
    vertexPositionBufferlayout.AddElement<float>(0, 2);

    // Texture coordinate
    vertexPositionBufferlayout.AddElement<float>(1, 2);

    particleVAO.AddBuffer(vertexPositionVBO, vertexPositionBufferlayout);



    constexpr std::uint32_t numberOfParticles = 250;

    // Particle opacity
    VertexBuffer opacityVBO = VertexBuffer(nullptr, sizeof(float) * numberOfParticles);

    BufferLayout opacityBufferLayout;

    opacityBufferLayout.AddElement<float>(2, 1, 1);

    particleVAO.AddBuffer(opacityVBO, opacityBufferLayout);



    // Particle trajectory
    VertexBuffer particleTrajectoryPositionVBO = VertexBuffer(nullptr, sizeof(glm::vec2) * numberOfParticles);

    BufferLayout particleTrajectoryPositionBufferLayout;

    particleTrajectoryPositionBufferLayout.AddElement<float>(3, 2, 1);
    particleTrajectoryPositionBufferLayout.AddElement<float>(4, 2, 1);

    particleVAO.AddBuffer(particleTrajectoryPositionVBO, particleTrajectoryPositionBufferLayout);



    // Particle transforms
    constexpr float particleScaleFactor = 0.05f;

    const glm::mat4 particleTransfrom = glm::scale(glm::mat4(1.0f), { particleScaleFactor, particleScaleFactor, particleScaleFactor });


    VertexBuffer transformVBO = VertexBuffer(nullptr, sizeof(particleTransfrom) * numberOfParticles);

    BufferLayout transformBufferlayout;

    transformBufferlayout.AddElement<float>(4, 4, 1);
    transformBufferlayout.AddElement<float>(5, 4, 1);
    transformBufferlayout.AddElement<float>(6, 4, 1);
    transformBufferlayout.AddElement<float>(7, 4, 1);

    particleVAO.AddBuffer(transformVBO, transformBufferlayout);



    // For now, create a default particle texture
    const Texture particleTexture = Texture("Resources\\Particle.png");

    // A shader program that will be used by the Particle emitter
    const ShaderProgram texturedShaderProgram = ShaderProgram("TexturedVertexShader.glsl", "TexturedFragmentShader.glsl");



    std::mt19937 rng = std::mt19937(std::random_device {}());

    // These values are completley arbitrary
    const std::uniform_real_distribution rateDistribution = std::uniform_real_distribution<float>(10.1f, 30.0f);

    const std::uniform_real_distribution trajectoryADistribution = std::uniform_real_distribution<float>(0.01f, 0.1f);
    const std::uniform_real_distribution trajectoryBDistribution = std::uniform_real_distribution<float>(4.4f, 4.5f);

    const std::uniform_real_distribution opacityDecreaseRateDistribution = std::uniform_real_distribution<float>(0.1f, 0.4f);

    // A list of particle emmiters
    std::vector<ParticleEmmiter> particleEmmiters = std::vector<ParticleEmmiter>();


    // Add a new particle emmiter on the mouse's position
    leftMouseButtonClickedCallback = [&]()
    {
        return;
        const auto mouseNDC = MouseToNDC() / particleScaleFactor;

        particleEmmiters.push_back(ParticleEmmiter(numberOfParticles,
                                   particleScaleFactor,
                                   // Translate the original particle transform to Mouse position
                                   glm::translate(particleTransfrom, { mouseNDC.x, mouseNDC.y, 0 }),
                                   texturedShaderProgram,
                                   particleVAO,
                                   particleTexture,
                                   vertexPositionVBO,
                                   transformVBO,
                                   opacityVBO,
                                   rng,
                                   rateDistribution,
                                   trajectoryADistribution,
                                   trajectoryBDistribution,
                                   opacityDecreaseRateDistribution));
    };


    // Destory all particle emitters
    rightMouseButtonClickedCallback = [&]()
    {
        return;
        for (ParticleEmmiter& particleEmmiter : particleEmmiters)
        {
            particleEmmiter.Destory();
        };
    };



    const std::uniform_int_distribution particleXDistribution = std::uniform_int_distribution(0, WindowWidth);
    const std::uniform_int_distribution particleYDistribution = std::uniform_int_distribution(0, WindowHeight);


    for (std::size_t i = 0; i < 120; i++)
    {
        const auto emitterPosition = ScreenToNDC({ particleXDistribution(rng), particleYDistribution(rng) }) / particleScaleFactor;

        particleEmmiters.push_back(ParticleEmmiter(numberOfParticles,
                                   particleScaleFactor,
                                   glm::translate(particleTransfrom, { emitterPosition.x, emitterPosition.y, 0 }),
                                   texturedShaderProgram,
                                   particleVAO,
                                   particleTexture,
                                   vertexPositionVBO,
                                   transformVBO,
                                   opacityVBO,
                                   rng,
                                   rateDistribution,
                                   trajectoryADistribution,
                                   trajectoryBDistribution,
                                   opacityDecreaseRateDistribution));
    };




    std::chrono::steady_clock::time_point timePoint1;
    std::chrono::steady_clock::time_point timePoint2;

    // Elapsed time between frames
    std::chrono::duration<float> elapsedTime = std::chrono::duration<float>(0);

    // Number of frames elapsed
    std::uint32_t elapsedFrames = 0;

    // Time change between timePoint2 and timePoint1
    std::chrono::duration<float> delta;


    // How often to display FPS
    constexpr auto fpsDisplayInterval = std::chrono::milliseconds(700);


    while (glfwWindowShouldClose(glfwWindow) == false)
    {
        timePoint1 = std::chrono::steady_clock::now();

        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // Bind, update, and draw, particles
        auto iterator = particleEmmiters.begin();

        while (iterator != particleEmmiters.cend())
        {
            ParticleEmmiter& particleEmmiter = *iterator;

            // If an emitter was destroyed...
            if (particleEmmiter.GetDestroyed() == true)
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
        if (elapsedTime > fpsDisplayInterval)
        {
            float fps = elapsedFrames / elapsedTime.count();

            elapsedFrames = 0;
            elapsedTime = std::chrono::milliseconds(0);

            char tileBuffer[64] { 0 };


            sprintf_s(tileBuffer, sizeof(tileBuffer), "Emmiters: %d, Particles: %d, FPS: %.2f", static_cast<int>(particleEmmiters.size()), static_cast<int>(particleEmmiters.size() * numberOfParticles), fps);

            // Display FPS
            glfwSetWindowTitle(glfwWindow, tileBuffer);
        };

    };


    glfwDestroyWindow(glfwWindow);
};
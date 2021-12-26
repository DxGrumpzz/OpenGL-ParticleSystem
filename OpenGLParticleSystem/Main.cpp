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



std::uint32_t MouseX = 0;
std::uint32_t MouseY = 0;

int WindowWidth = 0;
int WindowHeight = 0;

std::function<void(void)> leftMouseButtonClickedCallback;


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

    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height)
    {
        WindowWidth = width;
        WindowHeight = height;

        glViewport(0, 0, width, height);
    });

    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow*, int mouseButton, int action, int modifierBits)
    {
        if ((mouseButton == GLFW_MOUSE_BUTTON_1) &&
            (action == GLFW_PRESS))
        {
            leftMouseButtonClickedCallback();
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





constexpr float Fx(const float x, float a = 1.0f, float b = 1.0f)
{
    return -a * (x * x) + (b * x);
};



struct Particle
{
    float TrajectoryA = 0.0f;
    float TrajectoryB = 0.0f;

    glm::vec2 Trajectory = glm::vec2(1.0f);

    float Rate = 0.0f;

    glm::mat4 Transform = glm::mat4(1.0f);
};


class ParticleEmmiter
{
private:

    std::reference_wrapper<const ShaderProgram> _shaderProgram;
    std::mt19937& _rng;

    std::reference_wrapper<const std::uniform_real_distribution<float>> _rateDistribution;
    std::reference_wrapper<const std::uniform_real_distribution<float>> _trajectoryADistribution;
    std::reference_wrapper<const std::uniform_real_distribution<float>> _trajectoryBDistribution;

    glm::mat4 _globalParticleTransform;
    glm::mat4 _particleActiveTransform;
    float _particleScaleFactor;

    std::uint32_t _particleVAO;
    std::uint32_t _textureID;
    std::uint32_t _particleVertexPositionVBO;
    std::uint32_t _particleTransformVBO;

    std::vector<Particle> _particles;


public:

    ParticleEmmiter(const std::uint32_t numberOfParticles,
                    const float particleScaleFactor,
                    const glm::mat4& particleTransform,
                    const ShaderProgram& shaderProgram,
                    const std::uint32_t particleVAO,
                    const std::uint32_t textureID,
                    const std::uint32_t particleVertexPositionVBO,
                    const std::uint32_t particleTransformVBO,
                    std::mt19937& rng,
                    const std::uniform_real_distribution<float>& rateDistribution,
                    const std::uniform_real_distribution<float>& trajectoryADistribution,
                    const std::uniform_real_distribution<float>& trajectoryBDistribution) :
        _shaderProgram(shaderProgram),
        _rng(rng),
        _rateDistribution(rateDistribution),
        _trajectoryADistribution(trajectoryADistribution),
        _trajectoryBDistribution(trajectoryBDistribution),
        _globalParticleTransform(particleTransform),
        _particleActiveTransform(glm::mat4(1.0f)),
        _particleScaleFactor(particleScaleFactor),
        _particleVAO(particleVAO),
        _textureID(textureID),
        _particleVertexPositionVBO(particleVertexPositionVBO),
        _particleTransformVBO(particleTransformVBO)
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

        glBindVertexArray(_particleVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureID);

        glBindBuffer(GL_ARRAY_BUFFER, _particleVertexPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, _particleTransformVBO);
    };

    void Update(const float deltaTime)
    {
        for (std::size_t index = 0;
             Particle & particle : _particles)
        {
            particle.Trajectory.x += particle.Rate * deltaTime;
            particle.Trajectory.y = Fx(particle.Trajectory.x, particle.TrajectoryA, particle.TrajectoryB);


            const glm::vec2 ndcPosition = CartesianToNDC({ particle.Trajectory.x, particle.Trajectory.y }) / _particleScaleFactor;
            const auto transfromCopy = glm::translate(_globalParticleTransform, { ndcPosition.x , ndcPosition.y, 0.0f }) * particle.Transform;

            const glm::vec3 screenPosition = glm::vec3(transfromCopy[3]);

            if (screenPosition.y < -1.0f)
            {
                particle.Transform = _particleActiveTransform;

                particle.Trajectory = glm::vec2(0.0f);

                InitializeParticleValues(particle, index);
            };

            glBufferSubData(GL_ARRAY_BUFFER, sizeof(_globalParticleTransform) * index, sizeof(_globalParticleTransform), glm::value_ptr(transfromCopy));

            index++;
        };
    };

    void Draw() const
    {
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<std::uint32_t>(_particles.size()));
    };


    glm::mat4& GetGlobalParticleTransform()
    {
        return _globalParticleTransform;
    };

    glm::mat4& GetParticleActiveTransform()
    {
        return _particleActiveTransform;
    };


private:


    void InitializeParticleValues(Particle& particle, const std::size_t particleIndex)
    {
        const float newTrajectoryA = _trajectoryADistribution(_rng);
        float newTrajectoryB = _trajectoryBDistribution(_rng);

        if ((particleIndex & 1) == 1)
        {
            newTrajectoryB *= -1;
        };

        float newRate = _rateDistribution(_rng);

        if (std::signbit(newTrajectoryB) != std::signbit(newRate))
            newRate = -newRate;

        particle.TrajectoryA = newTrajectoryA;
        particle.TrajectoryB = newTrajectoryB;

        particle.Rate = newRate;
    };

};




int main()
{
    constexpr std::uint32_t initialWindowWidth = 800;
    constexpr std::uint32_t initialWindowHeight = 600;

    GLFWwindow* glfwWindow = InitializeGLFWWindow(initialWindowWidth, initialWindowHeight,
                                                  "OpenGL - Particle emmiter");

    SetupOpenGL();



    VertexArray vao = VertexArray();


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

    VertexBuffer vertexPositionVBO = VertexBuffer(&vertexPositions, sizeof(vertexPositions));

    BufferLayout vertexPositionBufferlayout;

    // Vertex position
    vertexPositionBufferlayout.AddElement<float>(0, 2);

    // Texture coordinate
    vertexPositionBufferlayout.AddElement<float>(1, 2);

    vao.AddBuffer(vertexPositionVBO, vertexPositionBufferlayout);



    constexpr std::uint32_t numberOfParticles = 250;

    constexpr float particleScaleFactor = 0.05f;

    glm::mat4 particleTransfrom = glm::scale(glm::mat4(1.0f), { particleScaleFactor, particleScaleFactor, particleScaleFactor });


    VertexBuffer transformVBO = VertexBuffer(nullptr, sizeof(particleTransfrom) * numberOfParticles);

    BufferLayout transformBufferlayout;

    transformBufferlayout.AddElement<float>(2, 4, 1);
    transformBufferlayout.AddElement<float>(3, 4, 1);
    transformBufferlayout.AddElement<float>(4, 4, 1);
    transformBufferlayout.AddElement<float>(5, 4, 1);

    vao.AddBuffer(transformVBO, transformBufferlayout);




    const std::uint32_t particleTextureID = GL::GenerateTexture("Resources\\Particle.png");


    const ShaderProgram shaderProgram = ShaderProgram("VertexShader.glsl", "FragmentShader.glsl");

    const ShaderProgram texturedShaderProgram = ShaderProgram("TexturedVertexShader.glsl", "TexturedFragmentShader.glsl");



    std::mt19937 rng = std::mt19937(std::random_device {}());

    const std::uniform_real_distribution rateDistribution = std::uniform_real_distribution<float>(15.1f, 30.0f);

    const std::uniform_real_distribution trajectoryADistribution = std::uniform_real_distribution<float>(0.01f, 0.1f);
    const std::uniform_real_distribution trajectoryBDistribution = std::uniform_real_distribution<float>(4.4f, 4.5f);



    std::vector<ParticleEmmiter> particleEmmiters = std::vector<ParticleEmmiter>();


    leftMouseButtonClickedCallback = [&]()
    {
        const auto mouseNDC = MouseToNDC() / particleScaleFactor;

        particleEmmiters.push_back(ParticleEmmiter(numberOfParticles,
                                   particleScaleFactor,
                                   glm::translate(particleTransfrom, { mouseNDC.x, mouseNDC.y, 0 }),
                                   texturedShaderProgram,
                                   vao.GetID(),
                                   particleTextureID,
                                   vertexPositionVBO.GetID(),
                                   transformVBO.GetID(),
                                   rng,
                                   rateDistribution,
                                   trajectoryADistribution,
                                   trajectoryBDistribution));
    };


    std::chrono::steady_clock::time_point timePoint1;
    std::chrono::steady_clock::time_point timePoint2;

    std::chrono::duration<float> elapsedTime = std::chrono::duration<float>(0);

    std::uint32_t elapsedFrames = 0;

    std::chrono::duration<float> delta;

    constexpr auto fpsDisplayInterval = std::chrono::milliseconds(700);



    while (glfwWindowShouldClose(glfwWindow) == false)
    {
        timePoint1 = std::chrono::steady_clock::now();

        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        for (ParticleEmmiter& particleEmmiter : particleEmmiters)
        {
            particleEmmiter.Bind();
            particleEmmiter.Update(delta.count());
            particleEmmiter.Draw();
        };



        glfwSwapBuffers(glfwWindow);


        timePoint2 = std::chrono::steady_clock::now();


        delta = timePoint2 - timePoint1;
        elapsedTime += std::chrono::duration<float>(delta);
        elapsedFrames++;


        if (elapsedTime > fpsDisplayInterval)
        {
            float fps = elapsedFrames / elapsedTime.count();

            elapsedFrames = 0;
            elapsedTime = std::chrono::milliseconds(0);

            char tileBuffer[16] { 0 };

            sprintf_s(tileBuffer, sizeof(tileBuffer), "FPS: %.2f", fps);

            glfwSetWindowTitle(glfwWindow, tileBuffer);
        };
    };


    glfwDestroyWindow(glfwWindow);
};
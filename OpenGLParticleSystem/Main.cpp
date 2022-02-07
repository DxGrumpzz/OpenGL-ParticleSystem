#define WIN32_LEAN_AND_MEAN
#define GLM_CONSTEXPR_SIMD

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include <chrono>
#include <functional>
#include <array>
#include <numbers>

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
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
        {
            if(source == GL_DEBUG_SOURCE_API &&
               type == GL_DEBUG_TYPE_PERFORMANCE)
            {
                std::cerr << message << "\n";
                break;
            };

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


/// <summary>
/// A simple parabolic trajectory function, returns the next 'y' position of a particle depending on it's 'x' position
/// </summary>
/// <param name="particleX">  </param>
/// <param name="a"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <param name="b"> No idea what this actually does, but it affects the trajectory somewhat </param>
/// <returns></returns>
constexpr float ParticleTrajectoryFunction(const float particleX, float a = 1.0f, float b = 1.0f)
{
    return particleX * (((-a) * particleX) + b);
};



/// <summary>
/// Read all text inside a file
/// </summary>
/// <param name="filename"> Path to sid file </param>
/// <returns></returns>
static std::string ReadAllText(const std::string& filename)
{
    // Open the file at the end so we can easily find its length
    std::ifstream fileStream = std::ifstream(filename, std::ios::ate);

    std::string fileContents;

    // Resize the buffer to fit content
    fileContents.resize(fileStream.tellg());

    fileStream.seekg(std::ios::beg);

    // Read file contents into the buffer
    fileStream.read(fileContents.data(), fileContents.size());

    return fileContents;
};


class ComputeShaderProgram
{
private:

    mutable std::unordered_map<std::string, std::uint32_t> _uniformLocations;

    std::uint32_t _programID = 0;

public:

    ComputeShaderProgram(const std::string_view& shaderPath)
    {
        const std::uint32_t computeShaderID = CreateAndCompileShader(shaderPath);

        _programID = CreateAndLinkProgram(computeShaderID);

        Bind();
    };

    ~ComputeShaderProgram()
    {
        if(_programID != 0)
            glDeleteProgram(_programID);
    };


public:

    void Bind() const
    {
        glUseProgram(_programID);
    };


    template<typename T>
    void SetUniformValue(const std::string_view& uniformName, const T& value) const
    {
        static_assert(false, "Unsupported type");
    };


    template<>
    void SetUniformValue(const std::string_view& uniformName, const float& value) const
    {
        Bind();

        const std::uint32_t uniformLocation = GetUniformLocation(uniformName.data());
        glUniform1f(uniformLocation, value);
    };


    template<>
    void SetUniformValue(const std::string_view& uniformName, const glm::mat4& value) const
    {
        Bind();

        const std::uint32_t uniformLocation = GetUniformLocation(uniformName.data());

        glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(value));
    };


    template<>
    void SetUniformValue(const std::string_view& uniformName, const std::uint32_t& value) const
    {
        Bind();

        const std::uint32_t uniformLocation = GetUniformLocation(uniformName.data());

        glUniform1ui(uniformLocation, value);
    };



    void Dispatch(const std::uint32_t dispatchGroupsX = 1, const std::uint32_t dispatchGroupsY = 1, const std::uint32_t dispatchGroupsZ = 1) const
    {
        Bind();

        glDispatchCompute(dispatchGroupsX, dispatchGroupsY, dispatchGroupsZ);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    };

public:

    std::uint32_t GetProgramID()const
    {
        return _programID;
    };


private:

    std::uint32_t CreateAndCompileShader(const std::string_view& shaderPath)
    {
        const std::uint32_t computeShaderID = glCreateShader(GL_COMPUTE_SHADER);

        const auto computeShaderSource = ReadAllText(shaderPath.data());


        const char* computeShaderSourcePointer = computeShaderSource.c_str();
        const int computeShaderSourceLength = static_cast<int>(computeShaderSource.length());

        glShaderSource(computeShaderID, 1, &computeShaderSourcePointer, &computeShaderSourceLength);
        glCompileShader(computeShaderID);


        // Ensure compilation is successful  
        int success = 0;
        glGetShaderiv(computeShaderID, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            int bufferLength = 0;
            glGetShaderiv(computeShaderID, GL_INFO_LOG_LENGTH, &bufferLength);

            std::string error;
            error.resize(bufferLength);

            glGetShaderInfoLog(computeShaderID, bufferLength, &bufferLength, error.data());


            std::cerr << "Compute shader compilation error:\n" << error << "\n";

            __debugbreak();
        };

        return computeShaderID;
    };

    std::uint32_t CreateAndLinkProgram(const std::uint32_t computeShaderID)
    {
        const std::uint32_t computeShaderProgramID = glCreateProgram();


        glAttachShader(computeShaderProgramID, computeShaderID);
        glLinkProgram(computeShaderProgramID);


        // Ensure linkage is successful
        int success = 0;
        glGetProgramiv(computeShaderProgramID, GL_LINK_STATUS, &success);

        if(!success)
        {
            int bufferLength = 0;
            glGetProgramiv(computeShaderProgramID, GL_INFO_LOG_LENGTH, &bufferLength);

            std::string error;
            error.resize(bufferLength);

            int bytesWritten = 0;
            glGetProgramInfoLog(computeShaderProgramID, bufferLength, &bytesWritten, error.data());

            std::cerr << "Program link error:\n" << error << "\n";

            __debugbreak();
        };

        glDeleteShader(computeShaderID);

        return computeShaderProgramID;
    };


    /// <summary>
    /// Find the location of a uniform
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    std::uint32_t GetUniformLocation(const std::string& name) const
    {
        // Check if uniform exists in cache
        const auto result = _uniformLocations.find(name);

        int uniformLocation = -1;

        // If name uniform isn't cached...
        if(result == _uniformLocations.cend())
        {
            // Find the uniform
            uniformLocation = glGetUniformLocation(_programID, name.c_str());

            if(uniformLocation == -1)
            {
                std::cerr << "Uniform location error: Unable to find \"" << name << "\"\n";
                __debugbreak();
            }
            else
            {
                // Add to cache
                _uniformLocations.insert(std::make_pair(name, static_cast<std::uint32_t>(uniformLocation)));
            };
        }
        // If uniform was found...
        else
        {
            uniformLocation = result->second;
        };

        return uniformLocation;
    };

};


class ShaderStorageBuffer
{
private:

    std::uint32_t _bufferId = 0;


public:


    ShaderStorageBuffer(const void* const bufferData, const std::size_t bufferSizeInBytes, const std::uint32_t bindIndex = 0, const std::uint32_t usageType = GL_STATIC_DRAW)
    {
        glGenBuffers(1, &_bufferId);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);

        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeInBytes, bufferData, usageType);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindIndex, _bufferId);
    };


    ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept :
        // Swap between this and the other buffer such that when we then call the destructor 
        // we call on a "null" object, which is fine apparently 
        _bufferId(other._bufferId)
    {
        // The "other" buffer is set to zero, because it will then be destroyed
        other._bufferId = 0;
    };


    ShaderStorageBuffer(const VertexBuffer&) = delete;


    ~ShaderStorageBuffer()
    {
        Destroy();
    };


public:

    void Bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    };

    template<typename T>
    void GetBuffer(T* bufferData, const std::size_t numberOfElementsInBuffer) const
    {
        Bind();

        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * numberOfElementsInBuffer, bufferData);
    };


    void Destroy() const
    {
        glDeleteBuffers(1, &_bufferId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    };


public:

    std::uint32_t GetBufferID() const
    {
        return _bufferId;
    };


public:

    ShaderStorageBuffer& operator = (const ShaderStorageBuffer&) = delete;


    ShaderStorageBuffer& operator = (ShaderStorageBuffer&& other) noexcept
    {
        if(this != &other)
        {
            std::swap(_bufferId, other._bufferId);

            other.Destroy();
            other._bufferId = 0;
        };

        return *this;
    };

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

    std::uint32_t TextureID = 0;
};


struct alignas(16) ComputeShaderParticle
{
    float TrajectoryA = 0.0f;
    float TrajectoryB = 0.0f;

    alignas(8) glm::vec2 Trajectory = glm::vec2(1.0f);

    glm::mat4 Transform = glm::mat4(1.0f);

    float Rate = 0.0f;
};



float RandomNumberGenerator(const glm::vec2& uv, float seed)
{
    const float fixedSeed = std::fabs(seed) + 1.0f;

    const float x = glm::dot(uv, glm::vec2(12.9898, 78.233) * fixedSeed);

    return glm::fract(glm::sin(x) * 43758.5453f);
};

float RandomNumberGenerator(const glm::vec2& uv, float seed, float min, float max)
{
    const float rng = RandomNumberGenerator(uv, seed);

    // Map [0, 1] to [min, max] 
    const float rngResult = min + rng * (max - min);

    return rngResult;
};


class ParticleEmmiter
{
private:

    std::uint32_t _numberOfParticles;

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

    std::vector<const Texture*> _particleTextures;

    std::reference_wrapper<const VertexBuffer> _particleVertexPositionVBO;
    std::reference_wrapper<const VertexBuffer> _particleTransformVBO;
    std::reference_wrapper<const VertexBuffer> _particleOpacityVBO;

    std::reference_wrapper<const ComputeShaderProgram> _computeShaderProgram;


    // std::reference_wrapper<const ShaderStorageBuffer> _inputParticleBuffer;
    // std::reference_wrapper<const ShaderStorageBuffer> _outputParticletBuffer;
    // std::reference_wrapper<const ShaderStorageBuffer> _outputParticleScreenTransformBuffer;

    ShaderStorageBuffer _inputParticleBuffer;
    ShaderStorageBuffer _outputParticletBuffer;
    ShaderStorageBuffer _outputParticleScreenTransformBuffer;


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
                    const std::vector<const Texture*>& textures,
                    const VertexBuffer& particleVertexPositionVBO,
                    const VertexBuffer& particleTransformVBO,
                    const VertexBuffer& particleOpacityVBO,
                    std::mt19937& rng,
                    const std::uniform_real_distribution<float>& rateDistribution,
                    const std::uniform_real_distribution<float>& trajectoryADistribution,
                    const std::uniform_real_distribution<float>& trajectoryBDistribution,
                    const std::uniform_real_distribution<float>& opacityDecreaseRateDistribution,
                    const ComputeShaderProgram& computeShaderProgram) :
        // const ShaderStorageBuffer& inputBuffer,
        // const ShaderStorageBuffer& outputBuffer,
        // const ShaderStorageBuffer& outputParticleScreenTransformBuffer) :
        _numberOfParticles(numberOfParticles),
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
        _particleTextures(textures),
        _particleVertexPositionVBO(particleVertexPositionVBO),
        _particleTransformVBO(particleTransformVBO),
        _particleOpacityVBO(particleOpacityVBO),
        _computeShaderProgram(computeShaderProgram),
        // _inputParticleBuffer(inputBuffer),
        // _outputParticletBuffer(outputBuffer),
        // _outputParticleScreenTransformBuffer(outputParticleScreenTransformBuffer)
        _inputParticleBuffer(nullptr, sizeof(ComputeShaderParticle)* numberOfParticles, 0, GL_DYNAMIC_COPY),
        _outputParticletBuffer(nullptr, sizeof(ComputeShaderParticle)* numberOfParticles, 1),
        _outputParticleScreenTransformBuffer(nullptr, sizeof(glm::mat4)* numberOfParticles, 2),
        _particles(numberOfParticles)
    {

        for(std::size_t index = 0;
            Particle & particle : _particles)
        {
            InitializeParticleValues(particle, index);
            index++;
        };


        // Fill the initial input SSBO with particle data
        _inputParticleBuffer.Bind();

        for(std::size_t i = 0; i < _numberOfParticles; i++)
        {
            const ComputeShaderParticle temp =
            {
                .TrajectoryA = _particles[i].TrajectoryA,
                .TrajectoryB = _particles[i].TrajectoryB,

                .Trajectory = _particles[i].Trajectory,

                .Transform = _particles[i].Transform,

                .Rate = _particles[i].Rate,
            };

            glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderParticle) * i, sizeof(ComputeShaderParticle), &temp);
        };
    };



public:

    void Bind() const
    {
        _computeShaderProgram.get().Bind();
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleEmmiterTransform", _particleEmmiterTransform);
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleTransform", _particleTransform);

        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowWidth", WindowWidth);
        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowHeight", WindowHeight);

        _computeShaderProgram.get().SetUniformValue<float>("ParticleScaleFactor", _particleScaleFactor);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _inputParticleBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _outputParticletBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _outputParticleScreenTransformBuffer.GetBufferID());

        _shaderProgram.get().Bind();

        _particleVAO.get().Bind();

        _particleVertexPositionVBO.get().Bind();

        std::uint32_t index = 0;
        for(const Texture* particleTexture : _particleTextures)
        {
            particleTexture->Bind(index);

            std::string uniformName;
            uniformName.reserve(16);

            uniformName.append("Textures[").append(std::to_string(index)).append("]");
            _shaderProgram.get().SetInt(uniformName, index);

            index++;
        };
    };


    /*
    void Update(const float deltaTime)
    {
        for(Particle& particle : _particles)
        {
            constexpr float rateIncrease = 0.01f;

            // Negative
            if(std::signbit(particle.Rate) == true)
                particle.Rate -= rateIncrease;
            // Positive
            else
                particle.Rate += rateIncrease;


            // Calculate next trajectory position
            particle.Trajectory.x += particle.Rate * deltaTime;
            particle.Trajectory.y = ParticleTrajectoryFunction(particle.Trajectory.x, particle.TrajectoryA, particle.TrajectoryB);

            // Update opacity
            particle.Opacity -= particle.OpacityDecreaseRate * deltaTime;
        };



        std::vector<ComputeShaderParticle> InValues = std::vector<ComputeShaderParticle>(_numberOfParticles);

        for(std::size_t i = 0; i < _numberOfParticles; i++)
        {
            InValues[i].Transform = _particles[i].Transform;
            // InValues[i].Trajectory = _particles[i].Trajectory;
        };

        _inputParticleBuffer.get().Bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, InValues.size() * sizeof(ComputeShaderParticle), InValues.data());
        // _inputBuffer.get().GetBuffer(InValues.data(), _numberOfParticles);

        _computeShaderProgram.get().Dispatch((_numberOfParticles / 256) + 1);


        std::vector<glm::mat4> OutValues = std::vector<glm::mat4>(_numberOfParticles);

        _outputParticleScreenTransformBuffer.get().GetBuffer(OutValues.data(), _numberOfParticles);



        _shaderProgram.get().Bind();


        auto particleTransformBuffer = _particleTransformVBO.get().MapBuffer<glm::mat4>(GL::AccessType::WriteOnly);
        auto particleOpacitiesBuffer = _particleOpacityVBO.get().MapBuffer<float>(GL::AccessType::WriteOnly);


        auto iterator = _particles.begin();

        // When using a for-loop and requesting the Emmiter to be destroyed, particles sometimes flicker.
        // Using iterators fixes it, I'm guessing it's do to with indexing(?)
        std::size_t index = 0;
        while(iterator != _particles.cend())
        {
            Particle& particle = *iterator;

            // Copy new opacity value into VBO
            particleOpacitiesBuffer.get()[index] = particle.Opacity;


            const glm::vec2 ndcPosition = CartesianToNDC({ particle.Trajectory.x, particle.Trajectory.y }) / _particleScaleFactor;

            NdcPositions[index] = ndcPosition;

            // First apply the transform which translates the particle onto some screen position
            // const glm::mat4 screenTransfrom = glm::translate(_particleEmmiterTransform, { ndcPosition.x , ndcPosition.y, 0.0f })
            //     // Apply active particle transform
            //     * particle.Transform;


            const glm::mat4& screenTransfrom = OutValues[index];

            ScreenTransforms[index] = screenTransfrom;


            // Get translation components of the transform.
            // This works as long as we don't use non-uniform transformations
            const glm::vec3 screenPosition = glm::vec3(screenTransfrom[3]);


            // Copy particle-screen transform into VBO
            particleTransformBuffer.get()[index] = screenTransfrom;


            // If the particle is outside screen bounds..
            if((screenPosition.y < -1.0f) ||
               // Or if the particle is practically inivsible
               (particle.Opacity < 0.0f))
            {
                if(_desrtoyRequested == true)
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

        int _ = 0;
    };
    */



    void Update2(const float deltaTime)
    {
        _computeShaderProgram.get().SetUniformValue<float>("DeltaTime", deltaTime);

        _computeShaderProgram.get().Dispatch((_numberOfParticles / 256) + 1);


        // "Convert" the output SSBO to a VBO and bind to the vertex shader's transform vertex layout
        glBindBuffer(GL_ARRAY_BUFFER, _outputParticleScreenTransformBuffer.GetBufferID());

        constexpr std::uint32_t vertexTransformIndex = 4;

        glVertexAttribPointer(vertexTransformIndex + 0, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<const void*>(sizeof(glm::vec4) * 0));
        glEnableVertexAttribArray(vertexTransformIndex + 0);
        glVertexAttribDivisor(vertexTransformIndex + 0, 1);

        glVertexAttribPointer(vertexTransformIndex + 1, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<const void*>(sizeof(glm::vec4) * 1));
        glEnableVertexAttribArray(vertexTransformIndex + 1);
        glVertexAttribDivisor(vertexTransformIndex + 1, 1);

        glVertexAttribPointer(vertexTransformIndex + 2, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<const void*>(sizeof(glm::vec4) * 2));
        glEnableVertexAttribArray(vertexTransformIndex + 2);
        glVertexAttribDivisor(vertexTransformIndex + 2, 1);

        glVertexAttribPointer(vertexTransformIndex + 3, 4, GL_FLOAT, false, sizeof(glm::mat4), reinterpret_cast<const void*>(sizeof(glm::vec4) * 3));
        glEnableVertexAttribArray(vertexTransformIndex + 3);
        glVertexAttribDivisor(vertexTransformIndex + 3, 1);


        // Copy the contents of the output SSBO into the intput SSBO
        glBindBuffer(GL_COPY_READ_BUFFER, _outputParticletBuffer.GetBufferID());
        glBindBuffer(GL_COPY_WRITE_BUFFER, _inputParticleBuffer.GetBufferID());

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(ComputeShaderParticle) * _numberOfParticles);

    };



    void Draw() const
    {
        _shaderProgram.get().Bind();
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
        if(_desrtoyRequested == true)
        {
            return _particles.empty();
        };

        return false;
    };


private:

    void InitializeParticleValues(Particle& particle, const std::size_t particleIndex)
    {
        // I have to use a "generator" here, if I don't then all the particles will be generated with values that are too close to each other
        constexpr auto generateUV = []() -> glm::vec2
        {
            return { static_cast<float>(glfwGetTime()), 1.0f / static_cast<float>(glfwGetTime()) };
        };

        constexpr float rngSeed = 1.0f / std::numbers::pi_v<float>;


        const float newTrajectoryA = RandomNumberGenerator(generateUV(), rngSeed, 0.01f, 0.1f);


        // A very simple way of creating some trajectory direction variation
        const float newTrajectoryB = (particleIndex & 1) == 1 ?
            -RandomNumberGenerator(generateUV(), rngSeed, 4.4f, 4.5f) :
            RandomNumberGenerator(generateUV(), rngSeed, 4.4f, 4.5f);



        // const float newOpacityDecreaseRate = _opacityDecreaseRateDistribution(_rng.get());


        // Correct the rate depending on trajectory direction
        const float newRate = std::signbit(newTrajectoryB) == true ?
            // "Left" trajectory 
            -RandomNumberGenerator(generateUV(), rngSeed, 10.5f, 30.0f) :
            // "Right" trajectory
            RandomNumberGenerator(generateUV(), rngSeed, 10.5f, 30.0f);



        particle.TrajectoryA = newTrajectoryA;
        particle.TrajectoryB = newTrajectoryB;

        particle.Trajectory = glm::vec2(0.0f);
        particle.Rate = newRate;

        particle.Opacity = 1.0f;
        // particle.OpacityDecreaseRate = newOpacityDecreaseRate;


        // Apply custom particle transform
        particle.Transform = _particleTransform;
    };

};




int main()
{
    constexpr std::uint32_t initialWindowWidth = 800;
    constexpr std::uint32_t initialWindowHeight = 600;

    constexpr bool generateEmitters = false;
    constexpr std::uint32_t emittersToGenerate = 20;


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


    std::mt19937 rng = std::mt19937(std::random_device {}());

    // These values are completley arbitrary
    const std::uniform_real_distribution rateDistribution = std::uniform_real_distribution<float>(11.5f, 20.0f);

    const std::uniform_real_distribution trajectoryADistribution = std::uniform_real_distribution<float>(0.01f, 0.1f);
    const std::uniform_real_distribution trajectoryBDistribution = std::uniform_real_distribution<float>(4.4f, 4.5f);

    const std::uniform_real_distribution opacityDecreaseRateDistribution = std::uniform_real_distribution<float>(0.1f, 0.4f);

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
                                          rng,
                                          rateDistribution,
                                          trajectoryADistribution,
                                          trajectoryBDistribution,
                                          opacityDecreaseRateDistribution,
                                          computeShader);
            // inputParticleBuffer,
            // outputParticleBuffer,
            // particleScreenTransformBuffer);
        };


        // Destory all particle emitters
        rightMouseButtonClickedCallback = [&]()
        {
            for(ParticleEmmiter& particleEmmiter : particleEmmiters)
            {
                particleEmmiter.Destory();
            };
        };

    }
    else if constexpr(generateEmitters == true)
    {
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
                                          rng,
                                          rateDistribution,
                                          trajectoryADistribution,
                                          trajectoryBDistribution,
                                          opacityDecreaseRateDistribution,
                                          computeShader);
            // inputParticleBuffer,
            // outputParticleBuffer,
            // particleScreenTransformBuffer);
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
            particleEmmiter.Update2(delta.count());
            
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
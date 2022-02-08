#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ShaderProgram.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "Math.hpp"
#include "ShaderStorageBuffer.hpp"
#include "ComputeShaderProgram.hpp"


// Defined in Main.cpp
extern int WindowWidth;
extern int WindowHeight;



struct Particle
{
    /// <summary>
    /// The 'a' coefficient in a parabola
    /// </summary>
    float TrajectoryA = 0.0f;

    /// <summary>
    /// The 'b' coefficient  in a parabola
    /// </summary>
    float TrajectoryB = 0.0f;

    /// <summary>
    /// The position of this particle in it's trajectory path
    /// </summary>
    glm::vec2 Trajectory = glm::vec2(0.0f);

    /// <summary>
    /// The rate at which this particle will move
    /// </summary>
    float Rate = 0.0f;

    /// <summary>
    /// A transform which will modify how this particle looks
    /// </summary>
    glm::mat4 Transform = glm::mat4(1.0f);

    float Opacity = 0.0f;

    /// <summary>
    /// How fast will the particle's opacity decay
    /// </summary>
    float OpacityDecreaseRate = 0.0f;

    /// <summary>
    /// This particle's texture
    /// </summary>
    std::uint32_t TextureID = 0;
};


/// <summary>
/// The bare-minimm particle data that is needed to send to the compute shader
/// </summary>
struct alignas(16) ComputeShaderParticle
{
    float TrajectoryA = 0.0f;
    float TrajectoryB = 0.0f;

    alignas(8) glm::vec2 Trajectory = glm::vec2(1.0f);

    glm::mat4 Transform = glm::mat4(1.0f);

    float Rate = 0.0f;

    float Opacity = 0.0f;

    float OpacityDecreaseRate = 0.0f;
};


/// <summary>
/// An emitter of particles. 
/// </summary>
class ParticleEmmiter
{

private:

    /// <summary>
    /// The number of particles that will be drawn
    /// </summary>
    std::uint32_t _numberOfParticles;

    /// <summary>
    /// A reference to the shader program which will draw the particles
    /// </summary>
    std::reference_wrapper<const ShaderProgram> _particleShaderProgram;


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


    /// <summary>
    /// A VAO for the particles
    /// </summary>
    std::reference_wrapper<const VertexArray> _particleVAO;


    /// <summary>
    /// A list of particle textures
    /// </summary>
    std::vector<const Texture*> _particleTextures;


    /// <summary>
    /// A reference to VBO of particle vertex positions
    /// </summary>
    std::reference_wrapper<const VertexBuffer> _particleVertexPositionVBO;

    /// <summary>
    /// A reference to a VBO of per-instance particle transforms
    /// </summary>
    std::reference_wrapper<const VertexBuffer> _particleTransformVBO;


    /// <summary>
    /// A reference to a VBO of per-instance particle opacities
    /// </summary>
    std::reference_wrapper<const VertexBuffer> _particleOpacityVBO;

    /// <summary>
    /// A reference to a particle transform compute shader
    /// </summary>
    std::reference_wrapper<const ComputeShaderProgram> _computeShaderProgram;


    /// <summary>
    /// An input SSBO for particle data
    /// </summary>
    ShaderStorageBuffer _inputParticleBuffer;

    /// <summary>
    /// The resulting output data of particle data after the compute shader has run
    /// </summary>
    ShaderStorageBuffer _outputParticleBuffer;

    /// <summary>
    /// An output SSBO of particle screen transforms
    /// </summary>
    ShaderStorageBuffer _outputParticleScreenTransformBuffer;

    /// <summary>
    /// An output SSBO of particle opacities
    /// </summary>
    ShaderStorageBuffer _outputParticleOpacitiesBuffer;


    /// <summary>
    /// A list of particles
    /// </summary>
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
                    const ComputeShaderProgram& computeShaderProgram) :
        // const ShaderStorageBuffer& inputBuffer,
        // const ShaderStorageBuffer& outputBuffer,
        // const ShaderStorageBuffer& outputParticleScreenTransformBuffer) :
        _numberOfParticles(numberOfParticles),
        _particleShaderProgram(shaderProgram),
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
        _outputParticleBuffer(nullptr, sizeof(ComputeShaderParticle)* numberOfParticles, 1),
        _outputParticleScreenTransformBuffer(nullptr, sizeof(glm::mat4)* numberOfParticles, 2),
        _outputParticleOpacitiesBuffer(nullptr, sizeof(float)* numberOfParticles, 3),
        _particles(numberOfParticles)
    {

        // Initialize particles
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

                .Opacity = _particles[i].Opacity,
                .OpacityDecreaseRate = _particles[i].OpacityDecreaseRate,
            };

            glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderParticle) * i, sizeof(ComputeShaderParticle), &temp);
        };
    };



public:

    void Bind() const
    {
        _particleVAO.get().Bind();


        _computeShaderProgram.get().Bind();

        // Update compute shader uniforms
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleEmmiterTransform", _particleEmmiterTransform);
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleTransform", _particleTransform);

        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowWidth", WindowWidth);
        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowHeight", WindowHeight);

        _computeShaderProgram.get().SetUniformValue<float>("ParticleScaleFactor", _particleScaleFactor);

        // Bind SSBOs to their respective binding points
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _inputParticleBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _outputParticleBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _outputParticleScreenTransformBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _outputParticleOpacitiesBuffer.GetBufferID());


        _particleShaderProgram.get().Bind();


        _particleVertexPositionVBO.get().Bind();

        std::uint32_t index = 0;
        for(const Texture* particleTexture : _particleTextures)
        {
            particleTexture->Bind(index);

            std::string uniformName;
            uniformName.reserve(16);

            uniformName.append("Textures[").append(std::to_string(index)).append("]");
            _particleShaderProgram.get().SetInt(uniformName, index);

            index++;
        };
    };


    void Update(const float deltaTime)
    {
        _computeShaderProgram.get().SetUniformValue<float>("DeltaTime", deltaTime);

        _computeShaderProgram.get().Dispatch((_numberOfParticles / 64) + 1);


        // Convert the opacity SSBO to a VBO
        glBindBuffer(GL_ARRAY_BUFFER, _outputParticleOpacitiesBuffer.GetBufferID());

        glVertexAttribPointer(2, 1, GL_FLOAT, false, sizeof(float), 0);
        glVertexAttribDivisor(2, 1);



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
        glBindBuffer(GL_COPY_READ_BUFFER, _outputParticleBuffer.GetBufferID());
        glBindBuffer(GL_COPY_WRITE_BUFFER, _inputParticleBuffer.GetBufferID());

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(ComputeShaderParticle) * _numberOfParticles);
    };


    void Draw() const
    {
        _particleShaderProgram.get().Bind();
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

    /// <summary>
    /// Initialize a particle with some random data
    /// </summary>
    /// <param name="particle"></param>
    /// <param name="particleIndex"></param>
    void InitializeParticleValues(Particle& particle, const std::size_t particleIndex)
    {
        // I am not using C++'s random library because I want to match the expected results in GLSL


        // I have to use a "generator" here, if I don't then all the particles will be generated with values that are too close to each other
        constexpr auto generateUV = []() -> glm::vec2
        {
            return { static_cast<float>(glfwGetTime()), 1.0f / static_cast<float>(glfwGetTime()) };
        };

        const float rngSeed = static_cast<float>(glfwGetTime());


        const float newTrajectoryA = RandomNumberGenerator(generateUV(), rngSeed, 0.01f, 0.1f);


        // A very simple way of creating some trajectory direction variation
        const float newTrajectoryB = (particleIndex & 1) == 1 ?
            -RandomNumberGenerator(generateUV(), rngSeed, 4.4f, 4.5f) :
            RandomNumberGenerator(generateUV(), rngSeed, 4.4f, 4.5f);


        // Correct the rate depending on trajectory direction
        const float newRate = std::signbit(newTrajectoryB) == true ?
            // "Left" trajectory 
            -RandomNumberGenerator(generateUV(), rngSeed, 10.5f, 30.0f) :
            // "Right" trajectory
            RandomNumberGenerator(generateUV(), rngSeed, 10.5f, 30.0f);


        const float newOpacityDecreaseRate = RandomNumberGenerator(generateUV(), rngSeed, 0.05f, 0.1f);


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

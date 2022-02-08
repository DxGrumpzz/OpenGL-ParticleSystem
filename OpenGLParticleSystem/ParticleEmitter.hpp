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



extern int WindowWidth;
extern int WindowHeight;



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

    float Opacity = 0.0f;

    float OpacityDecreaseRate = 0.0f;

};


class ParticleEmmiter
{

private:

    std::uint32_t _numberOfParticles;

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

    std::reference_wrapper<const VertexArray> _particleVAO;

    std::vector<const Texture*> _particleTextures;

    std::reference_wrapper<const VertexBuffer> _particleVertexPositionVBO;
    std::reference_wrapper<const VertexBuffer> _particleTransformVBO;
    std::reference_wrapper<const VertexBuffer> _particleOpacityVBO;

    std::reference_wrapper<const ComputeShaderProgram> _computeShaderProgram;


    ShaderStorageBuffer _inputParticleBuffer;
    ShaderStorageBuffer _outputParticletBuffer;
    ShaderStorageBuffer _outputParticleScreenTransformBuffer;
    ShaderStorageBuffer _outputParticleOpacitiesBuffer;


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
        _outputParticletBuffer(nullptr, sizeof(ComputeShaderParticle)* numberOfParticles, 1),
        _outputParticleScreenTransformBuffer(nullptr, sizeof(glm::mat4)* numberOfParticles, 2),
        _outputParticleOpacitiesBuffer(nullptr, sizeof(float)* numberOfParticles, 3),
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

                .Opacity = _particles[i].Opacity,
                .OpacityDecreaseRate = _particles[i].OpacityDecreaseRate,
            };

            glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderParticle) * i, sizeof(ComputeShaderParticle), &temp);
        };
    };



public:

    void Bind() const
    {
        _computeShaderProgram.get().Bind();

        // Update compute shader uniforms
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleEmmiterTransform", _particleEmmiterTransform);
        _computeShaderProgram.get().SetUniformValue<glm::mat4>("ParticleTransform", _particleTransform);

        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowWidth", WindowWidth);
        _computeShaderProgram.get().SetUniformValue<std::uint32_t>("WindowHeight", WindowHeight);

        _computeShaderProgram.get().SetUniformValue<float>("ParticleScaleFactor", _particleScaleFactor);

        // Bind SSBOs to their respective binding points
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _inputParticleBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _outputParticletBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _outputParticleScreenTransformBuffer.GetBufferID());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _outputParticleOpacitiesBuffer.GetBufferID());


        _particleShaderProgram.get().Bind();

        _particleVAO.get().Bind();

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
        glBindBuffer(GL_COPY_READ_BUFFER, _outputParticletBuffer.GetBufferID());
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

    void InitializeParticleValues(Particle& particle, const std::size_t particleIndex)
    {
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


#pragma once

#include <unordered_map>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>


/// <summary>
/// A class that encapsulates the functionality of a compute shader
/// </summary>
class ComputeShaderProgram
{
private:

    /// <summary>
    /// A list of known uniform locations
    /// </summary>
    mutable std::unordered_map<std::string, std::uint32_t> _uniformLocations;

    /// <summary>
    /// The ID of this shader 
    /// </summary>
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


    /// <summary>
    /// Dispatch a compute shader work groups
    /// </summary>
    /// <param name="dispatchGroupsX"></param>
    /// <param name="dispatchGroupsY"></param>
    /// <param name="dispatchGroupsZ"></param>
    void Dispatch(const std::uint32_t dispatchGroupsX = 1, const std::uint32_t dispatchGroupsY = 1, const std::uint32_t dispatchGroupsZ = 1) const
    {
        // Ensure that the compute shader is bound
        Bind();

        // Dispatch the work
        glDispatchCompute(dispatchGroupsX, dispatchGroupsY, dispatchGroupsZ);

        // Wait until all data is written to the SSBO buffers
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    };

public:

    std::uint32_t GetProgramID()const
    {
        return _programID;
    };


private:


    /// <summary>
    /// Read all text inside a file
    /// </summary>
    /// <param name="filename"> Path to sid file </param>
    /// <returns></returns>
    std::string ReadAllText(const std::string& filename)
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
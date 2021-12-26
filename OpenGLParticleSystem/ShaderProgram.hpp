#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>


class ShaderProgram
{
public:

    std::uint32_t ProgramID { 0 };

    ShaderProgram(const std::string& vertexShaderPath,
                  const std::string& fragmentShaderPath)
    {
        const std::uint32_t vertexShaderID = CompileVertexShader(vertexShaderPath);
        const std::uint32_t fragmentShaderID = CompileFragmentShader(fragmentShaderPath);


        ProgramID = CreateAndLinkShaderProgram(vertexShaderID, fragmentShaderID);

        glDeleteShader(fragmentShaderID);
        glDeleteShader(vertexShaderID);
    };


    void Bind() const
    {
        glUseProgram(ProgramID);
    };


    void SetVector3(const std::string& name, const float value1, const float value2, const float value3) const
    {
        const std::uint32_t colourUniformLocation = GetUniformLocation(name);

        glUniform3f(colourUniformLocation, value1, value2, value3);
    };

    void SetVector3(const std::string& name, const glm::vec3& vector) const
    {
        SetVector3(name, vector.x, vector.y, vector.z);
    };

    void SetFloat(const std::string& name, const float& value) const
    {
        const std::uint32_t colourUniformLocation = GetUniformLocation(name);
        glUniform1f(colourUniformLocation, value);
    };

    void SetMatrix4(const std::string& name, const glm::mat4& matrix) const
    {
        const std::uint32_t colourUniformLocation = GetUniformLocation(name);

        glUniformMatrix4fv(colourUniformLocation, 1, false, glm::value_ptr(matrix));
    };

    void SetInt(const std::string& name, const int value) const
    {
        const std::uint32_t colourUniformLocation = GetUniformLocation(name);

        glUniform1i(colourUniformLocation, value);
    };

    void SetBool(const std::string& name, const bool value) const
    {
        SetInt(name, value);
    };

private:


    std::uint32_t CompileVertexShader(const std::string& filename)
    {
        const std::string vertexShaderSource = ReadAllText(filename);

        std::uint32_t vertexShaderID = 0;
        vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

        const char* vertexShaderSourcePointer = vertexShaderSource.c_str();
        const int vertexShaderSourceLength = static_cast<int>(vertexShaderSource.length());

        glShaderSource(vertexShaderID, 1, &vertexShaderSourcePointer, &vertexShaderSourceLength);
        glCompileShader(vertexShaderID);


        int success = 0;
        glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);


        if (!success)
        {
            int bufferLength = 0;
            glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &bufferLength);

            std::string error;
            error.resize(bufferLength);

            glGetShaderInfoLog(vertexShaderID, bufferLength, &bufferLength, error.data());


            std::cerr << "Vertex shader compilation error:\n" << error << "\n";

            __debugbreak();
        };

        return vertexShaderID;
    };


    std::uint32_t CompileFragmentShader(const std::string& filename)
    {
        const std::string fragmentShaderSource = ReadAllText(filename);

        std::uint32_t fragmentShaderID = 0;
        fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        const char* fragmentShaderSourcePointer = fragmentShaderSource.c_str();
        const int fragmentShaderSourceLength = static_cast<int>(fragmentShaderSource.length());

        glShaderSource(fragmentShaderID, 1, &fragmentShaderSourcePointer, &fragmentShaderSourceLength);
        glCompileShader(fragmentShaderID);


        int success = 0;
        glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);


        if (!success)
        {
            int bufferLength = 0;
            glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &bufferLength);

            std::string error;
            error.resize(bufferLength);

            glGetShaderInfoLog(fragmentShaderID, bufferLength, &bufferLength, error.data());

            std::cerr << "Fragment shader compilation error:\n" << error << "\n";

            __debugbreak();
        };

        return fragmentShaderID;
    };


    std::uint32_t CreateAndLinkShaderProgram(const std::uint32_t vertexShaderID, const std::uint32_t fragmentShaderID)
    {
        const std::uint32_t programID = glCreateProgram();

        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);

        int success = 0;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);

        if (!success)
        {
            int bufferLength = 0;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &bufferLength);

            std::string error;
            error.resize(bufferLength);

            glGetProgramInfoLog(programID, bufferLength, &bufferLength, error.data());

            throw error;
        };

        return programID;
    };


    std::uint32_t GetUniformLocation(const std::string& name) const
    {
        const int uniformLocation = glGetUniformLocation(ProgramID, name.c_str());

        if (uniformLocation == -1)
        {
            std::cerr << "Uniform location error: Unable to find \"" << name << "\"\n";
            __debugbreak();
        };

        return uniformLocation;
    };



    std::string ReadAllText(const std::string& filename)
    {
        std::ifstream fileStream = std::ifstream(filename, std::ios::ate);

        std::string fileContents;

        fileContents.resize(fileStream.tellg());

        fileStream.seekg(std::ios::beg);

        fileStream.read(fileContents.data(), fileContents.size());

        return fileContents;
    };

};

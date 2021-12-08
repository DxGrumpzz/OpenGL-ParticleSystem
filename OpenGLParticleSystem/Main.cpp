#define WIN32_LEAN_AND_MEAN

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static std::uint32_t MouseX = 0;
static std::uint32_t MouseY = 0;

static std::uint32_t WindowWidth = 0;
static std::uint32_t WindowHeight = 0;


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


GLFWwindow* InitializeGLFWWindow(int windowWidth, int windowHeight)
{
    glfwInit();

    glfwSetErrorCallback(GLFWErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* glfwWindow = glfwCreateWindow(windowWidth, windowHeight, "OpenGL - Minesweeper", nullptr, nullptr);

    WindowWidth = windowWidth;
    WindowHeight = windowHeight;

    glfwMakeContextCurrent(glfwWindow);

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height)
    {
        WindowWidth = width;
        WindowHeight = height;

        glViewport(0, 0, width, height);
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


std::uint32_t GenerateTexture(const std::string& texturePath)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    std::uint32_t textureID = 0;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);


    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);
    std::uint8_t* pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (pixels == nullptr)
    {
        const char* error = stbi_failure_reason();

        __debugbreak();
    };


    std::uint32_t format = 0;

    if (channels == 4)
        format = GL_RGBA;
    else if (channels == 3)
        format = GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);


    stbi_image_free(pixels);
    pixels = nullptr;
    glBindTexture(GL_TEXTURE_2D, 0);


    return textureID;
};


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


const float Fx(const float x, float b = 1.0f, const float xOffset = 0.0f, const float yOffset = 0.0f)
{
    return b * std::powf((x - xOffset), 2) + yOffset;
};




glm::vec2 CartesianToNDC(const glm::vec2& position)
{
    return
    {
        ((2.0f * position.x) / WindowWidth),
        ((2.0f * position.y) / WindowHeight),
    };
};


glm::vec2 ScreenToNDC(const glm::vec2& position)
{
    return
    {
        ((2.0f * position.x) / WindowWidth) - 1.0f,
        -(((2.0f * position.y) / WindowHeight) - 1.0f),
    };
};

glm::vec2 ScreenToCartesian(const glm::vec2& position)
{
    return
    {
        position.x - (WindowWidth / 2.0f),
        -position.y - (WindowHeight / 2.0f),
    };
};

glm::vec2 MouseToCartesian()
{
    return ScreenToCartesian({ MouseX, MouseY });
};



int main()
{

    constexpr std::uint32_t initialWindowWidth = 800;
    constexpr std::uint32_t initialWindowHeight = 600;

    GLFWwindow* glfwWindow = InitializeGLFWWindow(initialWindowWidth, initialWindowHeight);

    ScreenToNDC({ 0, 0 });



    SetupOpenGL();


    constexpr float vertices[] =
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


    std::uint32_t vao = 0;
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);


    glGenBuffers(1, &vao);
    glBindBuffer(GL_ARRAY_BUFFER, vao);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, reinterpret_cast<const void*>(sizeof(float) * 2));
    glEnableVertexAttribArray(1);



    glm::mat4 transfrom1 = glm::scale(glm::mat4(1.0f), { 0.1f, 0.1f, 0.1f });
    // glm::mat4 transfrom2 = transfrom1;
    glm::mat4 transfrom2 = transfrom1;


    std::uint32_t transformVBO = 0;
    glGenBuffers(1, &transformVBO);
    glBindBuffer(GL_ARRAY_BUFFER, transformVBO);

    // glBufferData(GL_ARRAY_BUFFER, sizeof(transfrom), glm::value_ptr(transfrom), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transfrom1) * 2, nullptr, GL_DYNAMIC_DRAW);


    glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(glm::vec4) * 4, 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(glm::vec4) * 4, reinterpret_cast<const void*>(sizeof(glm::vec4) * 1));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glVertexAttribPointer(4, 4, GL_FLOAT, false, sizeof(glm::vec4) * 4, reinterpret_cast<const void*>(sizeof(glm::vec4) * 2));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(glm::vec4) * 4, reinterpret_cast<const void*>(sizeof(glm::vec4) * 3));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);


    glBindBuffer(GL_ARRAY_BUFFER, transformVBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(transfrom1), glm::value_ptr(transfrom1));
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(transfrom2), sizeof(transfrom2), glm::value_ptr(transfrom2));




    const std::uint32_t effectTextureID = GenerateTexture("Resources\\Effect.png");


    const ShaderProgram shaderProgram = ShaderProgram("VertexShader.glsl", "FragmentShader.glsl");

    const ShaderProgram texturedShaderProgram = ShaderProgram("TexturedVertexShader.glsl", "TexturedFragmentShader.glsl");




    while (glfwWindowShouldClose(glfwWindow) == false)
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        texturedShaderProgram.Bind();

        glBindTexture(GL_TEXTURE_2D, effectTextureID);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glBindBuffer(GL_ARRAY_BUFFER, transformVBO);

        
        glm::vec2 cartesianMouse = MouseToCartesian();

        const float value = -Fx(static_cast<float>(cartesianMouse.x), 0.01f);

        const glm::vec2 ndcValue = CartesianToNDC({ cartesianMouse.x, value });


        const auto transfrom2Copy = glm::translate(transfrom2, { ndcValue.x , ndcValue.y, 0.0f });
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(transfrom2), sizeof(transfrom2), glm::value_ptr(transfrom2Copy));

        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 2);

        glfwSwapBuffers(glfwWindow);
    };

    glfwDestroyWindow(glfwWindow);
};
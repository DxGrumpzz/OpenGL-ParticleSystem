#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TextureCoordinate;
layout(location = 2) in mat4 Transform;


out vec2 VertexShaderTextureCoordinateOutput;


void main()
{
    VertexShaderTextureCoordinateOutput = TextureCoordinate;

    gl_Position = Transform * vec4(Position, 0.0f, 1.0f);
};
#version 430 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TextureCoordinate;
layout(location = 2) in float Opacity;

layout(location = 3) in uint TextureUnit;

layout(location = 4) in mat4 Transform;





out vec2 VertexShaderTextureCoordinateOutput;
out float VertexShaderOpacityOutput;
flat out uint VertexShaderTextureUnitOutput;



void main()
{
    VertexShaderTextureCoordinateOutput = TextureCoordinate;
    
    // VertexShaderOpacityOutput = Opacity;
    VertexShaderOpacityOutput = 1.0f;
    
    VertexShaderTextureUnitOutput = TextureUnit;



    gl_Position = Transform * vec4(Position, 0.0f, 1.0f);
};
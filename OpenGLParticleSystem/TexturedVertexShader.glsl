#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 TextureCoordinate;
layout(location = 2) in float Opacity;

layout(location = 3) in vec2 ParticleTrajectoryPosition;

layout(location = 4) in mat4 Transform;



out vec2 VertexShaderTextureCoordinateOutput;
out float VertexShaderOpacityOutput;


// uniform int WindowHeight;
// uniform int WindowHeight;

/*
vec2 CartesianToNDC(inout vec2 cartesianPosition)
{
    return vec2(((2.0f * cartesianPosition.x) / WindowWidth), ((2.0f * cartesianPosition.y) / WindowHeight));
};
*/


void main()
{
    VertexShaderTextureCoordinateOutput = TextureCoordinate;
    VertexShaderOpacityOutput = Opacity;

    // vec2 ndcPosition = CartesianToNDC(ParticleTrajectoryPosition) / _particleScaleFactor;

    // int s = test;


    gl_Position = Transform * vec4(Position, 0.0f, 1.0f);
};
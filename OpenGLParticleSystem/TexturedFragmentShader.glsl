#version 430 core


in vec2 VertexShaderTextureCoordinateOutput;
in float VertexShaderOpacityCoordinateOutput;

uniform sampler2D Texture;

out vec4 OutputColour;


void main()
{
    // Subtract 1 from opacity so we subtract the correct quantity from the pixel's alpha
    const float opacityCopy = 1.0f - clamp(VertexShaderOpacityCoordinateOutput, 0.0f, 1.0f);

    OutputColour = texture(Texture, VertexShaderTextureCoordinateOutput);

    OutputColour.a = max(OutputColour.a - opacityCopy, 0.0f);
};
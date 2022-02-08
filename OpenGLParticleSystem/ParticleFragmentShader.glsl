#version 430 core

in vec2 VertexShaderTextureCoordinateOutput;
in float VertexShaderOpacityOutput;
flat in uint VertexShaderTextureUnitOutput;


uniform sampler2D Textures[3];

out vec4 OutputColour;


void main()
{
    OutputColour = texture(Textures[VertexShaderTextureUnitOutput], VertexShaderTextureCoordinateOutput);
    
    // Subtract 1 from opacity so we subtract the correct quantity from the pixel's alpha
    const float opacity = 1.0f - clamp(VertexShaderOpacityOutput, 0.0f, 1.0f);

    OutputColour.a = clamp(OutputColour.a - opacity, 0.0f, 1.0f);
};
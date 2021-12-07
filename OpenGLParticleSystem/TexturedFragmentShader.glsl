#version 330 core


in vec2 VertexShaderTextureCoordinateOutput;

uniform sampler2D Texture;

out vec4 OutputColour;

void main()
{
    OutputColour = texture(Texture, VertexShaderTextureCoordinateOutput);
};
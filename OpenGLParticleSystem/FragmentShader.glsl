#version 330 core

out vec4 OutputColour;

uniform vec3 InputColour;

void main()
{
    OutputColour = vec4(InputColour, 1.0f);
};
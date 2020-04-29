#version 440

layout (location = 0) in vec2 v_TexCoords;
layout (location = 1) in vec4 v_Color;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = v_Color;
}
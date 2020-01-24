#version 440

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform RenderData
{
    vec4 Color;
} u_RenderData;

void main()
{
    FragColor = u_RenderData.Color;
}
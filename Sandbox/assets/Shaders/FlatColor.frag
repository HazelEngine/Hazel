#version 440

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform RenderData
{
    vec3 Color;
} u_RenderData;

void main()
{
    FragColor = vec4(u_RenderData.Color, 1.0f);
}
#version 440

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

layout (location = 0) out vec4 v_Color;

layout (binding = 0) uniform SceneData
{
    mat4 ViewProjection;
    mat4 Transform;
} u_SceneData;

void main()
{
    v_Color = a_Color;
    gl_Position = u_SceneData.ViewProjection * u_SceneData.Transform * vec4(a_Position, 1.0f);
}
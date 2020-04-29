#version 440

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in vec4 a_Color;

layout (location = 0) out vec2 v_TexCoords;
layout (location = 1) out vec4 v_Color;

layout (set = 0, binding = 0) uniform SceneData
{
    mat4 ViewProj;
    mat4 Transform;
} u_SceneData;

void main()
{
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
    gl_Position = u_SceneData.ViewProj * u_SceneData.Transform * vec4(a_Position, 1.0f);
}
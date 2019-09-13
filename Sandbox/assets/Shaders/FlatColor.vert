#version 440

layout (location = 0) in vec3 a_Position;

layout (binding = 0) uniform SceneData
{
    mat4 ViewProjection;
    mat4 Transform;
} u_SceneData;

void main()
{
    gl_Position = u_SceneData.ViewProjection * u_SceneData.Transform * vec4(a_Position, 1.0f);
}
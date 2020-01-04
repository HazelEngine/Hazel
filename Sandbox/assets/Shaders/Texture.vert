#version 440
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

layout (location = 0) out vec2 v_TexCoord;

layout (binding = 0) uniform SceneData
{
    mat4 ViewProjection;
    mat4 Transform;
} u_SceneData;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_SceneData.ViewProjection * u_SceneData.Transform * vec4(a_Position, 1.0f);
}
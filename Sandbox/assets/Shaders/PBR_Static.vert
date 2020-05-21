#version 440
			
layout (location = 0) in vec3  a_Position;
layout (location = 1) in vec3  a_Normal;
layout (location = 2) in vec3  a_Tangent;
layout (location = 3) in vec3  a_Binormal;
layout (location = 4) in vec2  a_TexCoord;

layout (location = 0) out vec3  v_WorldPosition;
layout (location = 1) out vec3  v_Normal;
layout (location = 2) out vec3  v_Binormal;
layout (location = 3) out vec2  v_TexCoord;
layout (location = 4) out mat3  v_WorldNormals;

layout (binding = 0) uniform SceneData
{
    mat4 ViewProj;
    mat4 Transform;
} u_SceneData;

void main()
{
    vec4 localPosition = vec4(a_Position, 1.0f);

    v_WorldPosition = vec3(u_SceneData.Transform * localPosition);
    v_Normal = a_Normal;
    v_Binormal = a_Binormal;
    v_WorldNormals = mat3(u_SceneData.Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
    v_TexCoord = a_TexCoord;

    gl_Position = u_SceneData.ViewProj * u_SceneData.Transform * localPosition;
}
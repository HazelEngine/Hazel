#version 440
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;

layout (location = 0) out vec4 v_Color;
layout (location = 1) out vec2 v_TexCoord;

layout (binding = 0) uniform SceneData
{
    mat4 ViewProjection;
} u_SceneData;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    gl_Position = u_SceneData.ViewProjection * vec4(a_Position, 1.0f);
}
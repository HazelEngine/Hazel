#version 440
			
layout (location = 0) in vec3  a_Position;
layout (location = 1) in vec2  a_TexCoord;
layout (location = 2) in vec4  a_Color;
layout (location = 3) in float a_TexIndex;
layout (location = 4) in float a_TilingFactor;

layout (location = 0) out vec2  v_TexCoord;
layout (location = 1) out vec4  v_Color;
layout (location = 2) out float v_TexIndex;
layout (location = 3) out float v_TilingFactor;

layout (set = 0, binding = 0) uniform SceneData
{
    mat4 ViewProj;
    mat4 Transform;
} u_SceneData;

void main()
{
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
    gl_Position = u_SceneData.ViewProj * u_SceneData.Transform * vec4(a_Position, 1.0f);
}
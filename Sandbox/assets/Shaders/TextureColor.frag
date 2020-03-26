#version 440

layout (location = 0) in vec4 v_Color;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform RenderData
{
	float TilingFactor;
} u_RenderData;

layout (binding = 1) uniform sampler2D u_Texture;

void main()
{
    //FragColor = vec4(texture(u_Texture, v_TexCoord * u_RenderData.TilingFactor));
    //FragColor *= u_RenderData.Color;
    FragColor = v_Color;
}
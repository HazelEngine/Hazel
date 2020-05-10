#version 440

layout (location = 0) in vec2 v_TexCoord;
layout (location = 1) in vec4 v_Color;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform texture2D u_Texture;
layout (binding = 2) uniform sampler u_Sampler;

layout (binding = 3) uniform MaterialData
{
    vec4 Albedo;
} u_MaterialData;

void main()
{
    FragColor = texture(sampler2D(u_Texture, u_Sampler), v_TexCoord);
    FragColor * v_Color;
}
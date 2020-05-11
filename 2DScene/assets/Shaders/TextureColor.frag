#version 440

layout (location = 0) in vec2  v_TexCoord;
layout (location = 1) in vec4  v_Color;
layout (location = 2) in float v_TexIndex;
layout (location = 3) in float v_TilingFactor;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform texture2D u_Texture[32];
layout (binding = 2) uniform sampler u_Sampler;

void main()
{
    int texIndex = int(v_TexIndex);
    FragColor = texture(sampler2D(u_Texture[texIndex], u_Sampler), v_TexCoord * v_TilingFactor);
    FragColor *= v_Color;
}
#version 440

layout (location = 0) in vec4  v_Color;
layout (location = 1) in vec2  v_TexCoord;
layout (location = 2) in float v_TexIndex;
layout (location = 3) in float v_TilingFactor;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
    int texIndex = int(v_TexIndex);
    FragColor = texture(u_Textures[texIndex], v_TexCoord * v_TilingFactor);
    FragColor *= v_Color;
}
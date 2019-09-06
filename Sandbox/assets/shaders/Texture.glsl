#type vertex
#version 330 core
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
}

#type fragment
#version 330 core

in vec2 v_TexCoord;

layout (location = 0) out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_Color;

void main()
{
    FragColor = vec4(texture(u_Texture, v_TexCoord));
    FragColor.xyz *= u_Color;
}
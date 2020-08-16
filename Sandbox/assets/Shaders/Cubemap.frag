#version 440

layout (location = 0) in vec3 v_Position;

layout (location = 0) out vec4 FragColor;

layout (set = 1, binding = 0) uniform samplerCube u_Cubemap;

void main()
{
    FragColor = texture(u_Cubemap, v_Position);
}
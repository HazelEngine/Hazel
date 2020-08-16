#version 440
			
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3  v_Position;

layout (set = 0, binding = 0) uniform SceneData
{
    mat4 InverseViewProjection;
} u_SceneData;

void main()
{
    vec4 position = vec4(a_Position.xy, 0.0, 1.0);
    gl_Position = position;
    
    v_Position = (u_SceneData.InverseViewProjection * position).xyz;
}
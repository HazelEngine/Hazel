#version 440

layout (location = 0) in vec3  v_WorldPosition;
layout (location = 1) in vec3  v_Normal;
layout (location = 2) in vec3  v_Binormal;
layout (location = 3) in vec2  v_TexCoord;
layout (location = 4) in mat3  v_WorldNormals;

layout (location = 0) out vec4 FragColor;

layout (binding = 1) uniform MaterialData
{
    // Enable/disable textures
    bool EnableAlbedoTexture;
    bool EnableNormalTexture;
    bool EnableMetalnessRoughnessTexture;

    vec3  AlbedoColor;
    float Metalness;
    float Roughness;
} u_MaterialData;

layout (binding = 2) uniform ControlData
{
    int RenderMode;
} u_ControlData;

// PBR texture inputs
layout (set = 1, binding = 0) uniform texture2D u_AlbedoTexture;
layout (set = 2, binding = 0) uniform texture2D u_NormalTexture;
layout (set = 3, binding = 0) uniform texture2D u_MetalnessRoughnessTexture;
layout (binding = 3) uniform sampler u_Sampler;

void main()
{
    if (u_ControlData.RenderMode == 0)
        FragColor = texture(sampler2D(u_AlbedoTexture, u_Sampler), v_TexCoord);
    else if (u_ControlData.RenderMode == 1)
        FragColor = texture(sampler2D(u_NormalTexture, u_Sampler), v_TexCoord);
    else if (u_ControlData.RenderMode == 2)
        FragColor = texture(sampler2D(u_MetalnessRoughnessTexture, u_Sampler), v_TexCoord);
}
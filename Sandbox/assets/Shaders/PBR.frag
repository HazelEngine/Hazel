#version 440

// Inputs
layout (location = 0) in vec3  v_WorldPosition;
layout (location = 1) in vec3  v_Normal;
layout (location = 2) in vec3  v_Binormal;
layout (location = 3) in vec2  v_TexCoord;
layout (location = 4) in mat3  v_WorldNormals;

// Output
layout (location = 0) out vec4 FragColor;

// Constants
const float Pi = 3.141592;
const float Epsilon = 0.00001;

const int LightCount = 1;

// Constant normal incidence Fresnel factor for all dieletrics
const vec3 Fdieletric = vec3(0.04);

// Structs
struct Light
{
    vec3 Direction;
	float Padding;
    vec3 Radiance;
    float Multiplier;
};

// Uniforms
layout (binding = 1) uniform MaterialData
{
    // Enable/disable textures
    bool EnableAlbedoTexture;
    bool EnableNormalTexture;
    bool EnableRoughnessMetalnessTexture;

    float Metalness;
    vec3  AlbedoColor;
    float Roughness;
} u_MaterialData;

layout (binding = 2) uniform RenderData
{
    Light Lights;
    vec3  CameraPosition;
	float Padding;
} u_RenderData;

// PBR texture inputs
layout (set = 1, binding = 0) uniform texture2D u_AlbedoTexture;
layout (set = 2, binding = 0) uniform texture2D u_NormalTexture;
layout (set = 3, binding = 0) uniform texture2D u_RoughnessMetalnessTexture;
layout (binding = 3) uniform sampler u_Sampler;

struct PBRParameters
{
    vec3 Albedo;
    float Roughness;
    float Metalness;

    vec3 Normal;
    vec3 View;
    float NdotV;
} m_Params;

// Schlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NDFGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (Pi * denom * denom);
}

// Single term for separable Schlick-GGX below.
float GASchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GASchlickGGX(float cosLi, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remmaping for analytic lights.
    return GASchlickG1(cosLi, k) * GASchlickG1(NdotV, k);
}

vec3 Lighting(vec3 F0)
{
    vec3 result  = vec3(0.0);

    for (int i = 0; i < LightCount; i++)
    {
        vec3 Li = -u_RenderData.Lights.Direction;
        vec3 Lradiance = u_RenderData.Lights.Radiance * u_RenderData.Lights.Multiplier;
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3  F = FresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
        float D = NDFGGX(cosLh, m_Params.Roughness);
        float G = GASchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
        vec3 diffuseBRDF = kd * m_Params.Albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);

        result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

    return result;
}

void main()
{
    // Standard PBR Inputs
    m_Params.Albedo = u_MaterialData.EnableAlbedoTexture ?
        texture(sampler2D(u_AlbedoTexture, u_Sampler), v_TexCoord).rgb : u_MaterialData.AlbedoColor;
    m_Params.Metalness = u_MaterialData.EnableRoughnessMetalnessTexture ?
        texture(sampler2D(u_RoughnessMetalnessTexture, u_Sampler), v_TexCoord).b : u_MaterialData.Metalness;
    m_Params.Roughness = u_MaterialData.EnableRoughnessMetalnessTexture ?
        texture(sampler2D(u_RoughnessMetalnessTexture, u_Sampler), v_TexCoord).g : u_MaterialData.Roughness;
    m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight

    // Normals (either from vertex or map)
    m_Params.Normal = normalize(v_Normal);
    if (u_MaterialData.EnableNormalTexture)
    {
        m_Params.Normal = normalize(2.0 * texture(sampler2D(u_NormalTexture, u_Sampler), v_TexCoord).rgb - 1.0);
        m_Params.Normal = normalize(v_WorldNormals * m_Params.Normal);
    }

    m_Params.View = normalize(u_RenderData.CameraPosition - v_WorldPosition);
    m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    // Specular reflection vector
    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

    // Fresnel reflectance, metals use albedo
    vec3 F0 = mix(Fdieletric, m_Params.Albedo, m_Params.Metalness);

    vec3 lightContribution = Lighting(F0);
    vec3 iblContribution = vec3(1.0, 1.0, 1.0);

    FragColor = vec4(lightContribution, 1.0);
}
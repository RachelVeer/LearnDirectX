Texture2D txDiffuse[2] : register(t0);
SamplerState samLinear : register(s0);

static float4 PixelColor;

cbuffer Transform : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 viewPos;
}

struct Material
{
    float shininess;
    float padding[3];
};

cbuffer Material : register(b1)
{
    Material material;
}

struct DirLight
{
    float4 direction;
    
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

cbuffer DirLight : register(b2)
{
    DirLight dirLight;
}

struct PointLight
{
    float4 position;
    
    float constant;
    float linearL;
    float quadratic;
    float padding;
    
    float4 ambient;
    float4 diffuse;
    float4 specular;
};
#define NR_POINT_LIGHTS 4
cbuffer PointLight : register(b3)
{
    PointLight pointLights[NR_POINT_LIGHTS];
}

float4 CalcDirLight(DirLight light, float4 normal, float4 viewDir, float4 mDiffuse, float4 mSpecular);
float4 CalcPointLight(PointLight light, float4 normal, float4 worldPos, float4 viewDir, float4 mDiffuse, float4 mSpecular);

// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 normal : NORMAL, float4 worldPos : Position, float2 tex : TexCoord) : SV_TARGET
{   
    float4 mDiffuse = txDiffuse[0].Sample(samLinear, tex);
    float4 mSpecular = txDiffuse[1].Sample(samLinear, tex);
    // Properties
    float4 norm = normalize(normal);
    float4 viewDir = normalize(viewPos - worldPos);
    
    // Phase 1: Directional lighting
    float4 result = CalcDirLight(dirLight, norm, viewDir, mDiffuse, mSpecular);
    // Phase 2: Point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, worldPos, viewDir, mDiffuse, mSpecular);
    // Phase 3: Spot light
    // ...
    PixelColor = result;
    
    return saturate(PixelColor);
}

float4 CalcDirLight(DirLight light, float4 normal, float4 viewDir, float4 mDiffuse, float4 mSpecular)
{
    float4 lightDir = normalize(-light.direction);
    // Diffuse shading.
    float diff = max(dot(normal, lightDir), 0.0f);
    // Specular shading.
    float4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    // Combine results.
    float4 ambient = light.ambient * mDiffuse;
    float4 diffuse = light.diffuse * diff * mDiffuse;
    float4 specular = light.specular * spec * mSpecular;
    return (ambient + diffuse + specular);
}

float4 CalcPointLight(PointLight light, float4 normal, float4 worldPos, float4 viewDir, float4 mDiffuse, float4 mSpecular)
{
    float4 lightDir = normalize(light.position - worldPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - worldPos);
    float attenuation = 1.0 / (light.constant + light.linearL * 
                        distance + light.quadratic * (distance * distance));
    // combine results
    float4 ambient = light.ambient * mDiffuse;
    float4 diffuse = light.diffuse * diff * mDiffuse;
    float4 specular = light.specular * spec * mSpecular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
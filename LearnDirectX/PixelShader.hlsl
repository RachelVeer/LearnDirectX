Texture2D txDiffuse[2] : register(t0);
SamplerState samLinear : register(s0);

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

struct Light
{
    float4 position;
    
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float constant;
    float linearL;
    float quadratic;
    float padding;
};

cbuffer Light : register(b2)
{
    Light light;
}
// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 normal : NORMAL, float4 worldPos : Position, float2 tex : TexCoord) : SV_TARGET
{   
    float4 mDiffuse = txDiffuse[0].Sample(samLinear, tex);
    float4 mSpecular = txDiffuse[1].Sample(samLinear, tex);
    
    float distance = length(light.position - worldPos);
    float attenuation = 1.0f / (light.constant + light.linearL * distance +
                        light.quadratic * (distance * distance));
    
    // Ambient 
    float4 ambient = light.ambient * mDiffuse;
    
    // Diffuse
    float4 norm = normalize(normal);
    float4 lightDir = normalize(light.position - worldPos);
    float4 diff = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = light.diffuse * diff * mDiffuse;
    
    // Specular
    float4 viewDir = normalize(viewPos - worldPos);
    float4 reflectDir = reflect(-lightDir, norm);
    
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    float4 specular = light.specular * spec * mSpecular;
    
    ambient.xyz *= attenuation;
    diffuse.xyz *= attenuation;
    specular.xyz *= attenuation;
    
    float4 result = ambient + diffuse + specular;
    
    return saturate(result);
}
Texture2D txDiffuse : register(t0);
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
    float4 specular;
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
};

cbuffer Light : register(b2)
{
    Light light;
}
// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 normal : NORMAL, float4 worldPos : Position, float2 tex : TexCoord) : SV_TARGET
{   
    // Ambient 
    float4 ambient = light.ambient * float4(txDiffuse.Sample(samLinear, tex));
    
    // Diffuse
    float4 norm = normalize(normal);
    float4 lightDir = normalize(light.position - worldPos);
    float4 diff = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = light.diffuse * diff * float4(txDiffuse.Sample(samLinear, tex));
    
    // Specular
    float4 viewDir = normalize(viewPos - worldPos);
    float4 reflectDir = reflect(-lightDir, norm);
    
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    float4 specular = light.specular * (spec * material.specular);
    
    float4 result = ambient + diffuse + specular;
    
    return saturate(result);
}
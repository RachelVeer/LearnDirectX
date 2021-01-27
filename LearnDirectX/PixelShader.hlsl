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

struct Light
{
    float4 position;
    float4 direction;
    
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float constant;
    float linearL;
    float quadratic;
    float cutOff;
    float outerCutOff;
    float padding[3];
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
    
    // Spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);
    
    diffuse.xyz *= intensity;
    specular.xyz *= intensity;
    
    // Attenuation
    float distance = length(light.position - worldPos);
    float attenuation = 1.0f / (light.constant + light.linearL * distance +
                        light.quadratic * (distance * distance));
    ambient.xyz *= attenuation;
    diffuse.xyz *= attenuation;
    specular.xyz *= attenuation;
    
    float4 result = ambient + diffuse + specular;
    PixelColor = result;
    
    return saturate(PixelColor);
}
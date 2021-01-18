cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 lightPos;
    float4 viewPos;
    float4 objectColor;
    float4 lightColor;
}

// Position input (even if not utilized) to match vertex shader output. 
float4 PSmain(float4 pos : SV_POSITION, float4 color : COLOR, float4 normal : NORMAL) : SV_TARGET
{
    // Ambient 
    float4 ambientStrength = float4(0.1f, 0.1f, 0.1f, 1.0f);
    float4 ambient = ambientStrength * lightColor;
    
    // Diffuse
    float4 norm = normalize(normal);
    float4 lightDir = normalize(lightPos - color);
    float4 diff = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = diff * lightColor;
    
    // Specular
    float4 specularStrength = float4(0.9f, 0.9f, 0.9f, 0.9f);
    float4 viewDir = normalize(viewPos - color);
    float4 reflectDir = reflect(-lightDir, norm);
    
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), 16);
    float4 specular = specularStrength * spec * lightColor;
    
    float4 result = (ambient + diffuse + specular) * objectColor;
    
    return result;
}
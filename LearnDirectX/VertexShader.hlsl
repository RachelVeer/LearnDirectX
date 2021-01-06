struct VSOut
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

VSOut VSmain(float2 pos : POSITION, float2 tex : TEXCOORD)
{
    VSOut output;

    output.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
    output.tex = tex;

    return output;
}
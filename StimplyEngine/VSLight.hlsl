cbuffer LPos
{
    matrix lightPosMtx;
    float4 lightColor;
};

struct VSOut
{
    float3 lightColor : Color;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position)
{
    VSOut vout;
    vout.lightColor = lightColor;
    vout.pos = mul(float4(pos, 1.0f), lightPosMtx);
    return vout;
}
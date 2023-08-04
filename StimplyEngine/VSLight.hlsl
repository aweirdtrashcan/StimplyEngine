cbuffer LPos
{
    matrix model;
};

struct VSOut
{
    float4 lightColor : Color;
    float4 pspos : SV_Position;
};

VSOut main(float3 pos : Position)
{
    VSOut vout;
    vout.pspos = mul(float4(pos, 1.0f), model);
    vout.lightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return vout;
}
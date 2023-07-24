//SamplerState rSampler : register(s0);
//Texture2D tex;

struct InputData
{
//  float2 texCoord : TCoord;
    float4 color : Color;
};

float4 main(InputData id) : SV_TARGET
{
    return id.color;
}
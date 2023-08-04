cbuffer cbuf
{
    float4 lightColor;
};

float4 main() : SV_TARGET
{
    return lightColor;
}
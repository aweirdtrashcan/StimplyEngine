cbuffer CBufs
{
    float3 lightPos;
    float intensity;
};

static const float3 lightColor = { 1.0f, 1.0f, 0.6f };
static const float3 col = { 0.7f, 0.7f, 0.7f };
static const float c1 = 1.0f;
static const float c2 = 0.045f;
static const float c3 = 0.0075f;

float4 main(float3 normal : Normal, float3 worldPos : Pos) : SV_Target
{
    const float3 distanceToLight = (float3)lightPos - worldPos;
    const float3 ambient = col * 0.15f;
    const float lenghtToLight = length(distanceToLight);
    const float3 lightAngle = distanceToLight / lenghtToLight;
    const float attenuation = 1.0f / (c1 + c2 * lenghtToLight + c1 * (lenghtToLight * lenghtToLight));
    const float3 finalCol = col * intensity * attenuation * max(0.0f, dot(lightAngle, normal));
    return float4(saturate(finalCol + ambient), 1.0f);
}
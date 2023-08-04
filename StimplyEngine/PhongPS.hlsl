cbuffer LightCBuf
{
    float3 lightPos;
    float3 materialColor;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

float4 main(float3 normal : Normal, float3 worldPos : Pos) : SV_Target
{
    const float3 distanceToLight = (float3)lightPos - worldPos;
    const float lenghtToLight = length(distanceToLight);
    const float3 lightAngle = distanceToLight / lenghtToLight;
    const float attenuation = 1.0f / (attConst + attLin * lenghtToLight + attQuad * (lenghtToLight * lenghtToLight));
    const float3 diffuse = diffuseColor * diffuseIntensity * attenuation * max(0.0f, dot(lightAngle, normal));
    return float4((saturate(diffuse + ambient) * materialColor), 1.0f);
}
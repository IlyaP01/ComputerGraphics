TextureCube colorTexture : register (t0);
SamplerState colorSampler : register (s0);

struct VSOutput
{
     float4 position : SV_POSITION;
     float3 localPos : POSITION1;
};

float4 main(VSOutput input) : SV_TARGET
{
    return float4(colorTexture.Sample(colorSampler, input.localPos).xyz, 1.0f);
}
